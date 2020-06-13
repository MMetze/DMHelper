#ifndef DMCONSTANTS_H
#define DMCONSTANTS_H

#include <Qt>

//#define INCLUDE_NETWORK_SUPPORT
//#define INCLUDE_CHASE_SUPPORT

namespace DMHelper
{
    const int DMHELPER_MAJOR_VERSION = 1;
    const int DMHELPER_MINOR_VERSION = 7;
    const int DMHELPER_ENGINEERING_VERSION = 17;

    const int CAMPAIGN_MAJOR_VERSION = 2;
    const int CAMPAIGN_MINOR_VERSION = 0;

    const int BESTIARY_MAJOR_VERSION = 2;
    const int BESTIARY_MINOR_VERSION = 1;

    enum CampaignType
    {
        CampaignType_Base = 0,
        CampaignType_Campaign,
        CampaignType_Party,
        CampaignType_Combatant,
        CampaignType_Map,
        CampaignType_Text,
        CampaignType_Battle,
        CampaignType_BattleContent,
        CampaignType_ScrollingText,
        CampaignType_AudioTrack,
        CampaignType_WelcomeScreen,

        CampaignType_Placeholder = 0xffff
    };

    enum BattleEvent
    {
        BattleEvent_Blank = 0,
        BattleEvent_NewRound,
        BattleEvent_Damage
    };

    enum ChaseContent
    {
        ChaseContent_Blank = 0,
        ChaseContent_Complication,
        ChaseContent_Character
    };

    enum ActionType
    {
        ActionType_Base = -1,
        ActionType_Fill,
        ActionType_Path,
        ActionType_Point,
        ActionType_SetMarker,
        ActionType_Rect
    };

    enum CombatantType
    {
        CombatantType_Base = 0,
        CombatantType_Character,
        CombatantType_Monster,
        CombatantType_Reference     // In-battle reference to an NPC/Character
    };

    enum CombatantSize
    {
        CombatantSize_Unknown = 0,
        CombatantSize_Tiny,
        CombatantSize_Small,
        CombatantSize_Medium,
        CombatantSize_Large,
        CombatantSize_Huge,
        CombatantSize_Gargantuan,
        CombatantSize_Colossal
    };

    enum XPThreshold
    {
        XPThreshold_Easy = 0,
        XPThreshold_Medium,
        XPThreshold_Hard,
        XPThreshold_Deadly
    };

    enum TreeType
    {
        TreeType_Campaign = 0,
        TreeType_Notes,
        TreeType_Party_Title,
        TreeType_Character,
        TreeType_Adventure_Title,
        TreeType_Adventure,
        TreeType_Encounter_Title,
        TreeType_Encounter,
        TreeType_World,
        TreeType_Settings_Title,
        TreeType_Map_Title,
        TreeType_Map,
        TreeType_People_Title,
        TreeType_NPC,
        TreeType_AudioTrack
    };

    enum EditMode
    {
        EditMode_FoW = 0,
        EditMode_Edit,
        EditMode_Move,

        EditMode_Count
    };

    enum BrushType
    {
        BrushType_Circle = 0,
        BrushType_Square,
        BrushType_Select,

        BrushType_Count
    };

    enum BattleDialog_Z
    {
        BattleDialog_Z_Background = -4,
        BattleDialog_Z_Grid = -3,
        BattleDialog_Z_Camera = -2,
        BattleDialog_Z_BackHighlight = -1,
        BattleDialog_Z_Combatant = 0,
        BattleDialog_Z_SelectedCombatant = 1,
        BattleDialog_Z_FrontHighlight = 2,
        BattleDialog_Z_Overlay = 3
    };

    enum BattleFrameStates
    {
        BattleFrameState_CombatantEdit = 0,
        BattleFrameState_ZoomSelect,
        BattleFrameState_CameraSelect,
        BattleFrameState_CameraEdit,
        BattleFrameState_Distance,
        BattleFrameState_Pointer,
        BattleFrameState_FoWSelect,
        BattleFrameState_FoWEdit,
    };

    enum AudioType
    {
        AudioType_File = 0,
        AudioType_Syrinscape
    };

    const int TreeItemData_Type = Qt::UserRole + 1;
    const int TreeItemData_ID = Qt::UserRole + 2;
    const int TreeItemData_Object = Qt::UserRole + 3;

    const int ChaserContent_Type = Qt::UserRole + 1;
    const int ChaserContent_Pointer = Qt::UserRole + 2;

    const int CHARACTER_WIDGET_HEIGHT = 20;
    const int CHARACTER_ICON_WIDTH = CHARACTER_WIDGET_HEIGHT * 4 / 3;
    const int CHARACTER_ICON_HEIGHT = CHARACTER_WIDGET_HEIGHT * 2;

    const int STARTING_GRID_SCALE = 25;

    const int CURSOR_SIZE = 70; // increased, was 30

    const int ANIMATION_TIMER_DURATION = 25;
    const int ANIMATION_TIMER_PREVIEW = 5000;
    const int ANIMATION_TIMER_PREVIEW_FRAMES = ANIMATION_TIMER_PREVIEW / ANIMATION_TIMER_DURATION;

    const int XPThresholds[4][20] = {
        { 25,  50,  75, 125,  250,  300,  350,  450,  550,  600,  800, 1000, 1100, 1250, 1400, 1600, 2000, 2100,  2400,  2800},
        { 50, 100, 150, 250,  500,  600,  750,  900, 1100, 1200, 1600, 2000, 2200, 2500, 2800, 3200, 3900, 4200,  4900,  5700},
        { 75, 150, 225, 375,  750,  900, 1100, 1400, 1600, 1900, 2400, 3000, 3400, 3800, 4300, 4800, 5900, 6300,  7300,  8500},
        {100, 200, 400, 500, 1100, 1400, 1700, 2100, 2400, 2800, 3600, 4500, 5100, 5700, 6400, 7200, 8800, 9500, 10900, 12700}
    };

    const int XPProgression[20] = {
        0, 300, 900, 2700, 6500, 14000, 23000, 34000, 48000, 64000, 85000, 100000, 120000, 140000, 165000, 195000, 225000, 265000, 305000, 355000
    };

    const int ProficiencyProgression[20] = {
        2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6
    };
}

#endif // DMCONSTANTS_H
