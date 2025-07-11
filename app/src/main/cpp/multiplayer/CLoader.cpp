//
// Created on 24.01.2023.
//

#include <jni.h>
#include "CLoader.h"
#include "util/patch.h"
#include "crashlytics.h"
#include "CSettings.h"
#include "net/netgame.h"
#include "java_systems/Speedometr.h"
#include "java_systems/Donate.h"
#include "java_systems/ObjectEditor.h"
#include "java_systems/Styling.h"
#include "java_systems/TireShop.h"
#include "java_systems/casino/Dice.h"
#include "java_systems/TheftAuto.h"
#include "java_systems/casino/Baccarat.h"
#include "java_systems/Tab.h"
#include "java_systems/SelectEntity.h"
#include "java_systems/AdminRecon.h"
#include "java_systems/DailyReward.h"
#include "java_systems/AutoShop.h"
#include "java_systems/TechInspect.h"
#include "java_systems/AucContainer.h"
#include "java_systems/ChooseSpawn.h"
#include "java_systems/casino/LuckyWheel.h"
#include "java_systems/casino/Chip.h"
#include "java_systems/Treasure.h"
#include "gui/GiftNotify.h"
#include "java_systems/FurnitureFactory.h"
#include "java_systems/Samwill.h"
#include "java_systems/GunStore.h"
#include "java_systems/Achivments.h"
#include "java_systems/OilFactory.h"
#include "java_systems/FuelStation.h"
#include "java_systems/RadialMenu.h"
#include "java_systems/Authorization.h"
#include "java_systems/ArmyGame.h"
#include "java_systems/Medic.h"
#include "java_systems/Inventory.h"
#include "java_systems/GuiWrapper.h"
#include "java_systems/mine/MineGame1.h"
#include "java_systems/mine/MineGame2.h"
#include "java_systems/mine/MineGame3.h"
#include "java_systems/SkinShop.h"
#include "java_systems/Registration.h"
#include "java_systems/Taxi.h"
#include "java_systems/Race.h"
#include "java_systems/BattlePass.h"
#include "java_systems/Milk.h"
#include "GuiWrapper.h"
#include "MagicStore.h"
#include "SnapShotsWrapper.h"
#include "Monologue.h"
#include "MiningStore.h"
#include "BuyPlate.h"
#include "BusStation.h"
#include "WarPoints.h"
#include "BattlePassBuy.h"
#include "JavaGui.h"

void CLoader::loadBassLib()
{
   // LoadBassLibrary();
   // BASS_Init(-1, 44100, BASS_DEVICE_MONO | BASS_DEVICE_3D);
    //BASS_Set3DFactors(1, 0.15, 0);
    //BASS_Apply3D();
}

void CLoader::initCrashLytics()
{
    firebase::crashlytics::SetCustomKey("build data", __DATE__);
    firebase::crashlytics::SetCustomKey("build time", __TIME__);

    firebase::crashlytics::SetUserId(CSettings::m_Settings.szNickName);
    firebase::crashlytics::SetCustomKey("Nick", CSettings::m_Settings.szNickName);

    char str[100];

    sprintf(str, "0x%x", g_libGTASA);
    firebase::crashlytics::SetCustomKey("libGTASA.so", str);

    sprintf(str, "0x%x", g_libSAMP);
    firebase::crashlytics::SetCustomKey("libsamp.so", str);
//
//    sprintf(str, "0x%x", libc);
//    firebase::crashlytics::SetCustomKey("libc.so", str);
}

void CLoader::loadSetting()
{
    CSettings::LoadSettings(nullptr);
}

jclass LinkJavaClass(jclass localObj) {
    auto env = CJavaWrapper::GetEnv();
    auto globalRef = (jclass)env->NewGlobalRef(localObj);
    env->DeleteLocalRef(localObj);
    return globalRef;
}


void CLoader::initJavaClasses(JavaVM* pjvm) {
    JNIEnv* env = nullptr;
    (void)pjvm->GetEnv((void**)&env, JNI_VERSION_1_6);

    CJavaGui::clazz = LinkJavaClass(env->FindClass("com/game/russia/NewUiList"));

    CBattlePassBuy::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/battle_pass/BattlePassBuy"));
    CWarPoints::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/WarPoints"));
    CBusStation::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/BusStation"));
    CBattlePass::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/battle_pass/BattlePass"));
    CBuyPlate::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/BuyPlate"));
    CMiningStore::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/MiningStore"));
    CMonologue::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/Monologue"));
    SnapShotsWrapper::clazz = LinkJavaClass(env->FindClass("com/game/russia/EntitySnaps"));
    CRace::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/Race"));
    CMagicStore::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/magicStore/MagicStore"));
    CTaxi::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/Taxi"));
    CRegistration::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/Registration"));
    CSkinShop::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/SkinAndAcsShop"));
    CMineGame3::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/MineGame3"));
    CMineGame2::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/MineGame2"));
    CMineGame1::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/MineGame1"));
    CInventory::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/inventory/Inventory"));
    CMedic::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/PreDeath"));
    CArmyGame::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/ArmyGame"));
    CAuthorization::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/Authorization"));
    CRadialMenu::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/RadialMenu"));
    CFuelStation::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/FuelStation"));
    COilFactory::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/fillingGames/OilFactory"));
    CMilk::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/fillingGames/Milk"));
    CAchivments::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/achivments/Achivments"));
    CGunStore::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/GunShop"));
    CSamwill::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/Samwill"));
    CFurnitureFactory::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/FurnitureFactory"));
    CGiftNotify::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/GiftNotify"));
    CTreasure::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/treasure/Treasure"));
    CChip::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/casino/BuySellChip"));
    CLuckyWheel::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/casino/LuckyWheel"));
    CChooseSpawn::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/ChooseSpawn"));
    CAucContainer::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/AucContainer"));
    CTechInspect::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/TechIspect"));
    CAutoShop::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/AutoShop"));
    CDailyReward::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/DailyReward"));
    CAdminRecon::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/AdminRecon"));
    CTab::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/tab/Tab"));
    CSpeedometr::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/Speedometer"));
    CDonate::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/donate/Donate"));
    CObjectEditor::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/AttachEdit"));
    CStyling::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/styling/Styling"));
    CTireShop::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/tire_shop/TireShop"));
    CDice::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/casino/Dice"));
    CTheftAuto::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/theft_auto/TheftAuto"));
    CBaccarat::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/CasinoBaccarat"));
    CActionsPed::clazz = LinkJavaClass(env->FindClass("com/game/russia/gui/ActionsPed"));
}


