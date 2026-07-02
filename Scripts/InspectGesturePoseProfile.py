import unreal
import os


SLOT_NAME = "ActionSquadGesturePoseProfile"
USER_INDEX = 0
REPORT_PATH = os.path.join(unreal.Paths.project_saved_dir(), "GesturePoseProfileReport.txt")
REPORT_LINES = []


def describe_pose(label, has_flag, pose, encoded_pose):
    thumb = pose.get_editor_property("Thumb")
    index = pose.get_editor_property("Index")
    middle = pose.get_editor_property("Middle")
    ring = pose.get_editor_property("Ring")
    pinky = pose.get_editor_property("Pinky")
    encoded = encoded_pose or ""
    wrist = ""
    if " W" in encoded:
        wrist = encoded[encoded.rfind(" W"):].strip()
    line = (
        f"{label}: has={has_flag}, "
        f"thumb={thumb:.3f}, index={index:.3f}, middle={middle:.3f}, "
        f"ring={ring:.3f}, pinky={pinky:.3f}, encoded_len={len(encoded)}, wrist={wrist}"
    )
    REPORT_LINES.append(line)
    unreal.log(line)


if not unreal.GameplayStatics.does_save_game_exist(SLOT_NAME, USER_INDEX):
    REPORT_LINES.append(f"SaveGame not found: {SLOT_NAME}")
    unreal.log_warning(REPORT_LINES[-1])
else:
    profile = unreal.GameplayStatics.load_game_from_slot(SLOT_NAME, USER_INDEX)
    if not profile:
        REPORT_LINES.append(f"Could not load SaveGame: {SLOT_NAME}")
        unreal.log_warning(REPORT_LINES[-1])
    else:
        describe_pose(
            "SelectA",
            profile.get_editor_property("bHasSelectA"),
            profile.get_editor_property("SelectAPose"),
            profile.get_editor_property("SelectAEncodedPose"),
        )
        describe_pose(
            "SelectB",
            profile.get_editor_property("bHasSelectB"),
            profile.get_editor_property("SelectBPose"),
            profile.get_editor_property("SelectBEncodedPose"),
        )
        describe_pose(
            "Action",
            profile.get_editor_property("bHasAction"),
            profile.get_editor_property("ActionPose"),
            profile.get_editor_property("ActionEncodedPose"),
        )
        describe_pose(
            "FreeAttack",
            profile.get_editor_property("bHasWatch"),
            profile.get_editor_property("WatchPose"),
            profile.get_editor_property("WatchEncodedPose"),
        )
        describe_pose(
            "ProtectMe",
            profile.get_editor_property("bHasRecall"),
            profile.get_editor_property("RecallPose"),
            profile.get_editor_property("RecallEncodedPose"),
        )

with open(REPORT_PATH, "w", encoding="utf-8") as report:
    report.write("\n".join(REPORT_LINES))
    report.write("\n")
unreal.log(f"Wrote gesture profile report: {REPORT_PATH}")
