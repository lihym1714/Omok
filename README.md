# Frida Gadget 통합 가이드라인

## 개요
이 문서는 Android APK에 Frida Gadget을 삽입하여 런타임 후킹을 가능하게 하는 방법을 설명합니다. 특히 `AppComponentFactory`를 활용한 정교한 삽입 방식을 다룹니다.

## 사전 준비사항
- APK 디컴파일 도구 (apktool)
- Frida Gadget 라이브러리 파일들
- 텍스트 에디터
- Android 개발 환경

## 1단계: APK 디컴파일
```bash
apktool d your_app.apk -o your_app_decompiled
```

## 2단계: Assets 구조 설정

### 2.1 Assets 디렉토리 생성
```
your_app_decompiled/assets/
├── exploit.js
└── original.apk
```

### 2.2 exploit.js 스크립트 작성
```javascript
Java.perform(function () {
    var act = Java.use("com.yourpackage.YourMainActivity");
    act.yourMethod.implementation = function () {
        console.log("[HOOK] Method called -> returning true");
        return true; // 원하는 값으로 변경
    }
});

function native_log(tag, msg) {
    var tag_new = Memory.allocUtf8String(tag)
    var msg_new = Memory.allocUtf8String(msg)
    var param_type_list = ["int", "pointer", "pointer", "..."]
    var print_ptr = Module.getExportByName("liblog.so", "__android_log_print")
    const print = new NativeFunction(print_ptr, 'int', param_type_list)
    print(2, tag_new, msg_new)
}

native_log("TEST", "native log")
```

### 2.3 original.apk 파일 생성
```bash
# 빈 파일 또는 원본 APK 복사
touch your_app_decompiled/assets/original.apk
```

## 3단계: Native Library 설정

### 3.1 lib 디렉토리 구조
```
your_app_decompiled/lib/arm64-v8a/
├── libesst-gd.so
├── libesst-gd.config.so
├── libesst-gd_backup.so
├── libesst-gd15.so
├── libesst-gd16.so
├── libesst-gd17.so
└── libesst-gd2.so
```

### 3.2 라이브러리 파일 복사
```bash
mkdir -p your_app_decompiled/lib/arm64-v8a
cp gadget_libs/*.so your_app_decompiled/lib/arm64-v8a/
```

## 4단계: AppComponentFactory 생성

### 4.1 Esst.smali 파일 생성
`your_app_decompiled/smali/kr/co/everspin/Esst.smali` 파일을 생성합니다.

**주요 기능:**
- `AppComponentFactory`를 상속하여 앱 시작 시점에 실행
- `static constructor`에서 `esst-gd` 라이브러리 로드
- APK 내부에서 `assets/exploit.js`와 `assets/original.apk` 추출
- `/data/data/패키지명/` 경로에 파일 저장

### 4.2 패키지별 경로 수정
Esst.smali 내에서 다음 경로들을 실제 앱 패키지에 맞게 수정:

```smali
# pm path 명령어
const-string v19, "pm path com.yourpackage.name"

# 파일 저장 경로
const-string v18, "/data/data/com.yourpackage.name"
```

### 4.3 Scanner 호환성 처리
Android API 레벨 호환성을 위해 Scanner 생성자를 수정:

```smali
# 호환성 있는 방식
invoke-virtual {v12}, Ljava/lang/Process;->getInputStream()Ljava/io/InputStream;
move-result-object v3
new-instance v15, Ljava/util/Scanner;
invoke-direct {v15, v3}, Ljava/util/Scanner;-><init>(Ljava/io/InputStream;)V
```

## 5단계: AndroidManifest.xml 수정

### 5.1 AppComponentFactory 설정
```xml
<application 
    android:appComponentFactory="kr.co.everspin.Esst"
    android:extractNativeLibs="true"
    android:usesCleartextTraffic="true"
    ... >
```

### 5.2 필수 권한 추가
```xml
<uses-permission android:name="android.permission.INTERNET"/>
<uses-permission android:name="android.permission.ACCESS_NETWORK_STATE"/>
```

## 6단계: 빌드 및 테스트

### 6.1 APK 재빌드
```bash
apktool b your_app_decompiled -o your_app_hooked.apk
```

### 6.2 APK 서명
```bash
# 키스토어 생성 (처음인 경우)
keytool -genkey -v -keystore my-release-key.keystore -alias alias_name -keyalg RSA -keysize 2048 -validity 10000

# APK 서명
jarsigner -verbose -sigalg SHA1withRSA -digestalg SHA1 -keystore my-release-key.keystore your_app_hooked.apk alias_name

# APK 정렬
zipalign -v 4 your_app_hooked.apk your_app_hooked_aligned.apk
```

### 6.3 설치 및 테스트
```bash
adb install your_app_hooked_aligned.apk
adb logcat | grep -E "(FRIDA|HOOK|TEST)"
```

## 7단계: 후킹 스크립트 커스터마이징

### 7.1 메서드 후킹 예제
```javascript
Java.perform(function () {
    // 특정 클래스의 메서드 후킹
    var targetClass = Java.use("com.example.TargetClass");
    targetClass.targetMethod.implementation = function(param1, param2) {
        console.log("[HOOK] targetMethod called with:", param1, param2);
        
        // 원본 메서드 호출
        var result = this.targetMethod(param1, param2);
        
        // 결과 수정
        return modified_result;
    };
    
    // 정적 메서드 후킹
    targetClass.staticMethod.implementation = function(param) {
        console.log("[HOOK] staticMethod called");
        return true;
    };
});
```

### 7.2 네이티브 함수 후킹
```javascript
Java.perform(function () {
    // 네이티브 함수 후킹
    var nativeFunc = Module.getExportByName("libnative.so", "native_function");
    Interceptor.attach(nativeFunc, {
        onEnter: function(args) {
            console.log("[NATIVE] native_function called");
            this.arg0 = args[0];
        },
        onLeave: function(retval) {
            console.log("[NATIVE] native_function returned:", retval);
            retval.replace(ptr("0x1")); // 반환값 수정
        }
    });
});
```

## 8단계: 디버깅 및 문제 해결

### 8.1 일반적인 문제들

**문제 1: Scanner 생성자 오류**
```
java.lang.NoSuchMethodError: No direct method <init>(Ljava/io/Readable;)V
```
**해결책:** InputStream을 사용하는 Scanner 생성자로 변경

**문제 2: 패키지 경로 오류**
```
Failed to load /data/data/com.example.tester/exploit.js: No such file or directory
```
**해결책:** Esst.smali에서 패키지 경로를 실제 앱 패키지로 수정

**문제 3: 라이브러리 로드 실패**
**해결책:** 
- `android:extractNativeLibs="true"` 설정 확인
- lib 파일들이 올바른 아키텍처 폴더에 있는지 확인

### 8.2 로그 확인 방법
```bash
# Frida 관련 로그
adb logcat | grep -E "(FRIDA|frida)"

# 앱 패키지 로그
adb logcat | grep "com.yourpackage.name"

# 시스템 로그
adb logcat | grep -E "(AndroidRuntime|FATAL)"
```

## 9단계: 고급 활용

### 9.1 동적 스크립트 로딩
```javascript
// 런타임에 스크립트 파일 로드
Java.perform(function () {
    try {
        var File = Java.use("java.io.File");
        var scriptFile = File.$new("/data/data/com.yourpackage/script.js");
        if (scriptFile.exists()) {
            // 동적 스크립트 실행 로직
            console.log("Dynamic script found and loaded");
        }
    } catch (e) {
        console.log("Dynamic script loading failed:", e);
    }
});
```

### 9.2 조건부 후킹
```javascript
Java.perform(function () {
    var MainActivity = Java.use("com.example.MainActivity");
    MainActivity.onCreate.implementation = function(savedInstanceState) {
        // 특정 조건에서만 후킹
        if (some_condition) {
            console.log("[HOOK] Conditional hook triggered");
        }
        
        // 원본 메서드 호출
        this.onCreate(savedInstanceState);
    };
});
```

## 보안 고려사항

1. **프로덕션 환경 주의**: 개발/테스트 목적으로만 사용
2. **서명 키 관리**: 키스토어 파일을 안전하게 보관
3. **스크립트 보안**: 민감한 정보를 스크립트에 하드코딩하지 않음
4. **로그 정리**: 디버그 로그를 프로덕션에서 제거

## 참고 자료

- [Frida 공식 문서](https://frida.re/docs/)
- [APKTool 사용법](https://ibotpeaches.github.io/Apktool/)
- [Android AppComponentFactory](https://developer.android.com/reference/android/app/AppComponentFactory)

---

**작성일:** 2025년 9월 26일  
**버전:** 1.0  
**작성자:** Frida Gadget Integration Team
