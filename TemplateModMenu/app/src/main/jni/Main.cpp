#include <jni.h>                 // for JNI_ERR, JNIEnv, jclass, JNINativeM...
#include <pthread.h>             // for pthread_create
#include <unistd.h>              // for sleep
#include "Il2cpp/Il2cpp.h"       // for EnsureAttached, Init
#include "Il2cpp/il2cpp-class.h" // for Il2CppImage, Il2CppObject
#include "Includes/Logger.h"     // for LOGD, LOGI
#include "Includes/Utils.h"      // for isGameLibLoaded, isLibraryLoaded
#include "Includes/obfuscate.h"  // for make_obfuscator, OBFUSCATE
#include "Menu/Menu.h"           // for Icon, IconWebViewData, SettingsList
#include "Menu/Setup.h"          // for CheckOverlayPermission, Init
#include "Includes/Macros.h"

// Target lib here
#define targetLibName OBFUSCATE("libil2cpp.so")

Il2CppImage *g_Image = nullptr;
Il2CppImage *XDSDK = nullptr;
Il2CppImage *runTime = nullptr;

//Il2CppImage *unityCore = nullptr;

// exmaple dump.cs
// class Game.Sample.Class //Assembly-CSharp
// {
//     System.Int32 Id; // 0x10
//     System.Int32 BuffType; // 0x14
//     UnityEngine.Vector3 Vel; // 0x18
//     UnityEngine.Vector3 Pos // 0x24
//     System.Single Interval; // 0x30
//     System.Single Counter; // 0x34
//     System.Int32 GoIndex; // 0x38
//     System.String Param; // 0x40
//     System.Int32 get_Id();
//     System.Void set_Position(UnityEngine.Vector3 pos);
//     System.Void .ctor(); // 0x017ad8d4
// }
//
// class Game.Sample.Class.SubClass
// {
//     System.Void SampleMethod();
//     System.Void .ctor();
// }

bool enterMainScene = false;
bool forceUnpause = false;
bool returnToFoyer = false;
bool charSelect = false;
int32_t camScale = 1;
bool setCamScale = false;
bool loadSave = false;
bool saveGame = false;
bool quickRestart = false;

bool enableSheep = false;
bool enableAll = false;
bool enableCultist = false;

bool showLoginScreen = false;
bool enableEnglish = true;
int runSeed = -1;
bool getSeed = false;
bool setSeed = false;
int seedValue = 1;

bool initAmmonomicon = true;

bool updateUrl = false;

bool spawnCurrency = false;
int32_t currencyAmountToDrop = 0;
int32_t isMetaCurrency = 0;

bool currencyPickupModded = false;
bool addBlanks = false;

Il2CppObject *proOfflineCont = nullptr;
Il2CppObject *playerCtrlClass = nullptr;
Il2CppObject *cameraCtrlClass = nullptr;
//Il2CppObject *healthHaverClass = nullptr; // TODO: GET THE PLAYER INSTANCE
//Il2CppObject *playerConsClass = nullptr;

// HealthHaver (add health and shield controls (add/remove health/shields)
// HeartDispenser (change amount of stored hearts)
// PlayerConsumables (directly set shell amount - set_Currency) (JUST CHANGES THE TEXT)
// CurrencyPickup (change the value of currency pickups (regular/meta) ✅, Pickup(PlayerController player)
// LootEngine (spawn health/currency)
// PlayerController (set blank amount ✅, teleport to point)
// CameraController (for LootEngine player pos vector)
// Gun (add ammo, set to infinite ammo mode, bullet effects, etc.) TODO: TRY TO MAKE A DROPDOWN LIST WITH GUN NAMES (gun name <-> instance pointer)
// Add bossrush elevator controller (to disable bossrush payment)

struct Vector2
{
    float x, y;
};

struct Vector3
{
    float x, y, z;
};

struct Int32 { // TODO
    int32_t m_value;
};

enum ChallengeModeType
{
    None,
    ChallengeMode,
    ChallengeMegaMode,
    GunslingKingTemporary
};

struct QuickRestartOptions
{
    bool GunGame;
    ChallengeModeType ChallengeMode;
    bool BossRush;
    int NumMetas; // no idea what this is
};

void GameManager_Update(Il2CppObject *instance) {
    //LOGD("Update called");

    if (instance != nullptr) {
        if (enterMainScene) {
            instance->invoke_method<void>("EnterMainScene", 1);
            enterMainScene = false; // because we only want to call the method once
        }
        if (forceUnpause) {
            instance->invoke_method<void>("ForceUnpause");
            forceUnpause = false;
        }
        if (enableSheep) {
            auto gmClass = g_Image->getClass("GameManager");
            gmClass->invoke_static_method<void>("SetBaaBaaActive", 1);
            enableSheep = false;
        }
        if (charSelect) {
            instance->invoke_method<void>("LoadCharacterSelect", 1, 0, 0);
            charSelect = false;
        }
        if (returnToFoyer) {
            instance->invoke_method<void>("ReturnToFoyer");
            returnToFoyer = false;
        }
        if (setCamScale) {
            auto gmClass = g_Image->getClass("GameManager");
            LOGD("Camera Scale is", camScale);
            if (camScale == 1) {
                gmClass->invoke_static_method<void>("SetCameraScale", 1);
            } else if (camScale == 2) {
                gmClass->invoke_static_method<void>("SetCameraScale", 2);
            } else {
                gmClass->invoke_static_method<void>("SetCameraScale", 3);
            }
            setCamScale = false;
        }
        if (enableCultist) {
             if (proOfflineCont != nullptr) {
                LOGD("Found procedureOfflineContinue!");
                auto doubleStrategy = proOfflineCont->invoke_method<Il2CppObject*>("CreateLoginOfflineContinueStrategy", 2);
                doubleStrategy->invoke_method<void>("TryContinue");
            } else {
                LOGE("procedureOfflineContinue is null!");
            }
            enableCultist = false;
        }
        if (loadSave) {
            if (proOfflineCont != nullptr) {
                LOGD("Found procedureOfflineContinue!");
                auto singleStrategy = proOfflineCont->invoke_method<Il2CppObject*>("CreateLoginOfflineContinueStrategy", 1);
                singleStrategy->invoke_method<void>("TryContinue");
            } else {
                LOGE("procedureOfflineContinue is null!");
            }
            loadSave = false;
        }
        if (saveGame) {
            LOGD("Saved Game!");
            instance->invoke_method<void>("SaveMidGameData");
            saveGame = false;
        }
        if (quickRestart) {
            LOGD("Quick-restarting!");
            ChallengeModeType challengeModeType = None;
            QuickRestartOptions quickResOpt = {
                    false,
                    challengeModeType,
                    false,
                    0
            };
            instance->invoke_method<void>("QuickRestart", quickResOpt);
            quickRestart = false;
        }
        if (enableAll) {
            if (proOfflineCont != nullptr) {
                LOGD("Found procedureOfflineContinue!");
                auto doubleStrategy = proOfflineCont->invoke_method<Il2CppObject*>("CreateLoginOfflineContinueStrategy", 2);
                doubleStrategy->invoke_method<void>("HandleOfflineContinue");

            } else {
                LOGD("procedureOfflineContinue is null!");
            }
            enableAll = false;
        }
        if (spawnCurrency) {
            if (cameraCtrlClass != nullptr) {
                auto playerPos = cameraCtrlClass->invoke_method<Vector2>("GetPlayerPosition", playerCtrlClass);
                //auto *spawnPos = Il2cpp::NewVector2(40.0f, 40.0f);

                if (playerPos.x != NULL && playerPos.y != NULL) {
                    LOGD("Spawning currency at x=%f, y=%f: amount: %d, isMeta: %d",
                         playerPos.x,
                         playerPos.y,
                         currencyAmountToDrop,
                         isMetaCurrency);

                    auto leClass = g_Image->getClass("LootEngine");
                    leClass->invoke_static_method<void>("SpawnCurrency", playerPos, 10, 1); // FIXME: Still doesn't work
                    //auto spawnCurrencyMethod = leClass->getMethod("SpawnCurrency", 3);

                    //if (spawnCurrencyMethod != nullptr) {
                    //    spawnCurrencyMethod->invoke_static<void>(playerPos,
                    //                                             10,
                    //                                             1); // true
                    //} else {
                        //LOGE("Failed to find SpawnCurrency method!");
                    //}
                } else {
                    LOGE("playerPos is null!");
                }
            } else {
                LOGE("playerCtrlClass or cameraCtrlClass is null!");
            }
            spawnCurrency = false;
        }
//        if (addBlanks) {
//            if (playerCtrlClass != nullptr) {
//                playerCtrlClass->setField("Blanks", 15);
//            } else {
//                LOGE("playerCtrlClass is null!");
//            }
//
//            addBlanks = false;
//        }

        return instance->invoke_method<void>("Update");

    }
}

Il2CppObject* getSettingData(Il2CppObject *instance) {
    if (enableEnglish) {
        LOGD("ENGLISH APPLIED!");
        instance->invoke_method<void>("ApplyLanguage", 10);
        enableEnglish = false;
    }
    return instance->invoke_method<Il2CppObject*>("get_SettingData");

}


void OnLoginClick(Il2CppObject *instance) {
    LOGD("OnLoginClick called");
    enterMainScene = true;
    
}
/*
void WikiUpdate(Il2CppObject *instance) {
    if (updateUrl) {
        auto webViewClass = g_Image->getClass("UIUniWebView");
        if (webViewClass != nullptr) {
            auto field = webViewClass->getField("wikiUrl");
            if (field != nullptr) {
                auto newUrl = Il2cpp::NewString("https://enterthegungeon.wiki.gg/");
                field->setStaticValue(newUrl);
            }
        }
        updateUrl = false;
    }

    return instance->invoke_method<void>("Update");
}*/

void AmmonomiconLateUpdate(Il2CppObject *instance) {
    if (initAmmonomicon) {

        LOGD("PrecacheAllData");

        instance->invoke_method<void>("PrecacheAllData");
        initAmmonomicon = false;
    }

    return instance->invoke_method<void>("LateUpdate");
}

void InitSDK(Il2CppObject *instance, Il2CppObject *callback) {
    instance->invoke_method<void>("disableAgreementUI");

    return instance->invoke_method<void>("InitSDK", callback);
}

/*
void DoGameOver(Il2CppObject *instance, Il2CppObject *gameOverSource) {
    LOGD("DoGameOver");
    returnToFoyer = true;
}
*/
void (*o_Class_GameMain_ProcedureOfflineContinue_ctor)(Il2CppObject *);
void Class_GameMain_ProcedureOfflineContinue_ctor(Il2CppObject *instance)
{
    o_Class_GameMain_ProcedureOfflineContinue_ctor(instance);
    proOfflineCont = instance;
}

void (*o_Class_PlayerController_ctor)(Il2CppObject *);
void Class_PlayerController_ctor(Il2CppObject *instance)
{
    o_Class_PlayerController_ctor(instance);
    playerCtrlClass = instance;
}

void (*o_Class_CameraController_ctor)(Il2CppObject *);
void Class_CameraController_ctor(Il2CppObject *instance)
{
    o_Class_CameraController_ctor(instance);
    cameraCtrlClass = instance;
}

//void (*o_Class_PlayerConsumables_ctor)(Il2CppObject *);
//void Class_PlayerConsumables_ctor(Il2CppObject *instance)
//{
//    o_Class_PlayerConsumables_ctor(instance);
//    playerConsClass = instance;
//}

void CurrencyPickup_Update(Il2CppObject *instance)
{
    if (currencyPickupModded) {
        instance->setField("currencyValue", 999);
    }
    instance->invoke_method<void>("Update");
}

void DoEnableAll(Il2CppObject *instance) {
    LOGD("Enable All Stuff");
    instance->invoke_method<void>("OnBtnContinueClick");
    instance->invoke_method<void>("OnBtnLeaveClick");
    instance->invoke_method<void>("OnBtnLeaveClick");
    charSelect = true;

}

bool IsConnectedInternet() {
    LOGD("FORCE INTERNET");
    return true;
}


/*
Il2CppObject* FindProcedureOfflineContinue() {
    // Get the ProcedureOfflineContinue class
    auto objectClass = unityCore->getClass("UnityEngine.Object");
    if (objectClass == nullptr) {
        LOGD("Failed to find Object class");
        return nullptr;
    }

    auto procedureClass = g_Image->getClass("GameMain.ProcedureOfflineContinue");
    if (procedureClass == nullptr) {
        LOGD("Failed to find ProcedureOfflineContinue class");
        return nullptr;
    }

    // Get the method and inflate it with the generic type parameter
    auto method = objectClass->getMethod("FindFirstObjectByType", 0);
    if (method == nullptr) {
        LOGD("Failed to find FindFirstObjectByType method");
        return nullptr;
    }

    // Inflate the method with the generic type parameter
    auto inflatedMethod = method->inflate({procedureClass});
    if (inflatedMethod == nullptr) {
        LOGD("Failed to inflate FindFirstObjectByType method");
        return nullptr;
    }

    // Call the inflated method
    auto procedureInstance = inflatedMethod->invoke_static<Il2CppObject*>();
    
    if (procedureInstance == nullptr) {
        LOGD("No ProcedureOfflineContinue instance found");
        return nullptr;
    }

    return procedureInstance;
}
*/

/*
void Class_set_Position(Il2CppObject *instance, Vector3 pos)
{
    LOGD("set_Position: %f, %f, %f", pos.x, pos.y, pos.z);
    pos.x += 1;
    return instance->invoke_method<void>("set_Position", pos);
}



void (*o_Class_GameMain_ProcedureOfflineContinue_ctor)(Il2CppObject *);
void Class_GameMain_ProcedureOfflineContinue_ctor(Il2CppObject *instance)
{
    o_Class_GameMain_ProcedureOfflineContinue_ctor(instance);
    auto id = instance->getField<int32_t>("Id");
    LOGINT(id);
    instance->setField("BuffType", 2);
}

void SubClass_SampleMethod(Il2CppObject *instance)
{
    LOGD("SampleMethod");
    return instance->invoke_method<void>("SampleMethod");
}
*/

// we will run our hacks in a new thread so our while loop doesn't block process main thread
void *hack_thread(void *)
{
    LOGI(OBFUSCATE("pthread created"));

    // Check if target lib is loaded
    do
    {
        sleep(1);
    } while (!isLibraryLoaded(targetLibName));

    LOGI(OBFUSCATE("%s has been loaded"), (const char *)targetLibName);

    Il2cpp::Init();
    Il2cpp::EnsureAttached();

    LOGD(OBFUSCATE("HOOKING..."));
    g_Image = Il2cpp::GetAssembly("Assembly-CSharp")->getImage();
    XDSDK = Il2cpp::GetAssembly("XDSDK.Runtime")->getImage();
    runTime = Il2cpp::GetAssembly("XD.SDK.Common.Mobile.Runtime")->getImage();
    //unityCore = Il2cpp::GetAssembly("UnityEngine.CoreModule")->getImage();

    // // HOOKS
    REPLACE_NAME("GameManager", "Update", GameManager_Update);
    REPLACE_NAME("UILoginMenuWindow", "OnLoginClick", OnLoginClick);
//    REPLACE_NAME("PauseMenuController", "DoShowBestiary", DoShowBestiary);
//    REPLACE_NAME("GameManager", "DoGameOver", DoGameOver);
    REPLACE_NAME("SettingService", "get_SettingData", getSettingData);
    REPLACE_NAME_ORIG("GameMain.ProcedureOfflineContinue", ".ctor", Class_GameMain_ProcedureOfflineContinue_ctor, o_Class_GameMain_ProcedureOfflineContinue_ctor);
    REPLACE_NAME_ORIG("PlayerController", ".ctor", Class_PlayerController_ctor, o_Class_PlayerController_ctor);
    REPLACE_NAME_ORIG("CameraController", ".ctor", Class_CameraController_ctor, o_Class_CameraController_ctor);
//    REPLACE_NAME_ORIG("PlayerConsumables", ".ctor", Class_PlayerConsumables_ctor, o_Class_PlayerConsumables_ctor);
    REPLACE_NAME("CurrencyPickup", "Update", CurrencyPickup_Update);
    REPLACE_NAME("UIContinueConfirmWindow", "OnBtnLeaveClick", DoEnableAll);
    REPLACE_NAME("AmmonomiconController", "LateUpdate", AmmonomiconLateUpdate);
    REPLACE_NAME_KLASS(XDSDK->getClass(OBFUSCATE("XDSDKAgent.SDKEntry")), "IsConnectedInternet", IsConnectedInternet);
    REPLACE_NAME_KLASS(runTime->getClass(OBFUSCATE("XD.SDK.Common.XDGCommonMobileImpl")), "InitSDK", InitSDK);

//    REPLACE_NAME("UIUniWebView", "Update", WikiUpdate);

    //REPLACE_NAME_ORIG("Game.Sample.Class", ".ctor", Class_GameMain_ProcedureOfflineContinue_ctor, o_Class_GameMain_ProcedureOfflineContinue_ctor);

    // // HOOK SUBCLASS
    // auto SubClass = g_Image->getClass("Game.Sample.Class.SubClass", 1);
    // REPLACE_NAME_KLASS(SubClass, "SampleMethod", SubClass_SampleMethod);

    LOGD(OBFUSCATE("HOOKED!"));

    return nullptr;
}



jobjectArray GetFeatureList(JNIEnv *env, [[maybe_unused]] jobject context)
{
    jobjectArray ret;




    const char *features[] = {
        OBFUSCATE("Category_General"),

        OBFUSCATE("Button_Enter The Breach"), // 0
        OBFUSCATE("Button_Fix Screen Bugs"), // 1
        OBFUSCATE("Button_Load Character Select"), // 2
        OBFUSCATE("SeekBar_Camera Scale_1_3"), // 3
        OBFUSCATE("Button_Try Load Autosave"), // 4
        OBFUSCATE("Button_Save Mid Game"), // 5
        OBFUSCATE("Button_Quick Restart"), // 6

        OBFUSCATE("Category_Content"),

        OBFUSCATE("Button_Enable Cult of the Lamb Event"), // 7
        OBFUSCATE("Button_Enable All Characters"), // 8
        OBFUSCATE("Button_Enable Cultist"), // 9

        OBFUSCATE("Collapse_Extras"),

        OBFUSCATE("CollapseAdd_Category_Shells / Credits"),

        OBFUSCATE("CollapseAdd_Button_Spawn Currency"), // 10
        OBFUSCATE("CollapseAdd_InputValue_Amount To Drop"), // 11
        OBFUSCATE("CollapseAdd_Toggle_Meta Currency?"), // 12

        OBFUSCATE("CollapseAdd_Category_Other Consumables"),
        OBFUSCATE("CollapseAdd_Category_Testing"),
        OBFUSCATE("CollapseAdd_Toggle_Spawn Modded Currency (999 value)"), // 13
//        OBFUSCATE("CollapseAdd_Button_Set Blanks to 15") // 14
    };

    // Now you dont have to manually update the number everytime;
    int Total_Feature = (sizeof features / sizeof features[0]);
    ret = (jobjectArray)env->NewObjectArray(Total_Feature, env->FindClass(OBFUSCATE("java/lang/String")),
                                            env->NewStringUTF(""));

    for (int i = 0; i < Total_Feature; i++)
        env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));

    return (ret);
}

void Changes(JNIEnv *env, [[maybe_unused]] jclass clazz, [[maybe_unused]] jobject obj, jint featNum, jstring featName,
             jint value, jboolean boolean, jstring str)
{

    LOGD(OBFUSCATE("Feature name: %d - %s | Value: = %d | Bool: = %d | Text: = %s"), featNum,
         env->GetStringUTFChars(featName, nullptr), value, boolean,
         str != nullptr ? env->GetStringUTFChars(str, nullptr) : "");

    // BE CAREFUL NOT TO ACCIDENTALLY REMOVE break;

    switch (featNum)
    {
        case 0:
        {
            enterMainScene = true;
            enableEnglish = true;
            break;
        }
        case 1:
        {
            forceUnpause = true;
            enterMainScene = true;
            enableEnglish = true;
            break;
        }
        case 2:
        {
            charSelect = true;
            enableEnglish = true;
            break;
        }
        case 3:
        {
            camScale = value;
            setCamScale = true;
            break;
        }
        case 4:
        {
            loadSave = true;
            enableEnglish = true;
            break;
        }
        case 5:
        {
            saveGame = true;
            enableEnglish = true;
            break;
        }
        case 6:
        {
            quickRestart = true;
            break;
        }
        case 7:
        {
            enableSheep = true;
            break;
        }
        case 8:
        {
            enableAll = true;
            enableEnglish = true;
            break;
        }
        case 9:
        {
            enableCultist = true;
            enableEnglish = true;
            break;
        }
        case 10:
        {
            spawnCurrency = true;
            break;
        }
        case 11:
        {
            currencyAmountToDrop = value;
            break;
        }
        case 12:
        {
            isMetaCurrency = boolean;
            break;
        }
        case 13:
        {
            currencyPickupModded = boolean;
            break;
        }
//        case 14:
//        {
//            addBlanks = true;
//            break;
//        }
    }
}

__attribute__((constructor)) void lib_main()
{
    // Create a new thread so it does not block the main thread, means the game would not freeze
    pthread_t ptid;
    pthread_create(&ptid, nullptr, hack_thread, nullptr);
}

int RegisterMenu(JNIEnv *env)
{
    JNINativeMethod methods[] = {
        {OBFUSCATE("Icon"), OBFUSCATE("()Ljava/lang/String;"), reinterpret_cast<void *>(Icon)},
        {OBFUSCATE("IconWebViewData"), OBFUSCATE("()Ljava/lang/String;"), reinterpret_cast<void *>(IconWebViewData)},
        {OBFUSCATE("IsGameLibLoaded"), OBFUSCATE("()Z"), reinterpret_cast<void *>(isGameLibLoaded)},
        {OBFUSCATE("Init"), OBFUSCATE("(Landroid/content/Context;Landroid/widget/TextView;Landroid/widget/TextView;)V"),
         reinterpret_cast<void *>(Init)},
        {OBFUSCATE("SettingsList"), OBFUSCATE("()[Ljava/lang/String;"), reinterpret_cast<void *>(SettingsList)},
        {OBFUSCATE("GetFeatureList"), OBFUSCATE("()[Ljava/lang/String;"), reinterpret_cast<void *>(GetFeatureList)},
    };

    jclass clazz = env->FindClass(OBFUSCATE("com/android/support/Menu"));
    if (!clazz)
        return JNI_ERR;
    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) != 0)
        return JNI_ERR;
    return JNI_OK;
}

int RegisterPreferences(JNIEnv *env)
{
    JNINativeMethod methods[] = {
        {OBFUSCATE("Changes"), OBFUSCATE("(Landroid/content/Context;ILjava/lang/String;IZLjava/lang/String;)V"),
         reinterpret_cast<void *>(Changes)},
    };
    jclass clazz = env->FindClass(OBFUSCATE("com/android/support/Preferences"));
    if (!clazz)
        return JNI_ERR;
    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) != 0)
        return JNI_ERR;
    return JNI_OK;
}

int RegisterMain(JNIEnv *env)
{
    JNINativeMethod methods[] = {
        {OBFUSCATE("CheckOverlayPermission"), OBFUSCATE("(Landroid/content/Context;)V"),
         reinterpret_cast<void *>(CheckOverlayPermission)},
    };

    jclass clazz = env->FindClass(OBFUSCATE("com/android/support/Main"));
    if (!clazz)
        return JNI_ERR;
    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) != 0)
        return JNI_ERR;

    return JNI_OK;
}

extern "C" JNIEXPORT jint

    JNICALL
    JNI_OnLoad(JavaVM *vm, [[maybe_unused]] void *reserved)
{
    JNIEnv *env;
    vm->GetEnv((void **)&env, JNI_VERSION_1_6);
    if (RegisterMenu(env) != 0)
        return JNI_ERR;
    if (RegisterPreferences(env) != 0)
        return JNI_ERR;
    if (RegisterMain(env) != 0)
        return JNI_ERR;
    return JNI_VERSION_1_6;
}
