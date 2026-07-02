import os
import unreal


ASSET_DIR = "/Game/Blueprints/Tutorial"
LEVEL_DIR = "/Game/Level"
GESTURE_LEVEL = f"{LEVEL_DIR}/GestureRecorder"
TUTORIAL_LEVEL = f"{LEVEL_DIR}/Tutorial"
AUDIO_SRC = os.path.join(unreal.Paths.project_content_dir(), "Audio", "AS_Tutorial_Ambient_Placeholder.wav")
AUDIO_ASSET_PATH = "/Game/Audio/AS_Tutorial_Ambient_Placeholder"


def ensure_dir(path: str):
    unreal.EditorAssetLibrary.make_directory(path)


def create_blueprint(asset_name: str, parent_class_path: str):
    ensure_dir(ASSET_DIR)
    asset_path = f"{ASSET_DIR}/{asset_name}"
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        return unreal.EditorAssetLibrary.load_asset(asset_path)

    parent_class = unreal.load_class(None, parent_class_path)
    if not parent_class:
        raise RuntimeError(f"Could not load parent class: {parent_class_path}")

    factory = unreal.BlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(asset_name, ASSET_DIR, unreal.Blueprint, factory)
    if not asset:
        raise RuntimeError(f"Failed to create blueprint: {asset_path}")
    unreal.EditorAssetLibrary.save_loaded_asset(asset)
    return asset


def load_bp_class(asset_path: str):
    asset_name = asset_path.rsplit("/", 1)[-1]
    bp_class = unreal.load_class(None, f"{asset_path}.{asset_name}_C")
    if not bp_class:
        raise RuntimeError(f"Could not load generated BP class: {asset_path}")
    return bp_class


def set_label(actor, label: str, folder: str):
    actor.set_actor_label(label, mark_dirty=True)
    actor.set_folder_path(folder)


def import_audio():
    if not os.path.exists(AUDIO_SRC):
        unreal.log_warning(f"Audio source not found: {AUDIO_SRC}")
        return None

    if unreal.EditorAssetLibrary.does_asset_exist(AUDIO_ASSET_PATH):
        return unreal.EditorAssetLibrary.load_asset(AUDIO_ASSET_PATH)

    task = unreal.AssetImportTask()
    task.set_editor_property("filename", AUDIO_SRC)
    task.set_editor_property("destination_path", "/Game/Audio")
    task.set_editor_property("automated", True)
    task.set_editor_property("save", True)
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return unreal.EditorAssetLibrary.load_asset(AUDIO_ASSET_PATH)


def destroy_labeled(labels):
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        if actor.get_actor_label() in labels:
            unreal.EditorLevelLibrary.destroy_actor(actor)


def setup_tutorial_navigation_and_audio(sound_asset):
    unreal.EditorLevelLibrary.load_level(TUTORIAL_LEVEL)
    destroy_labeled({"AS_NavMeshBounds_TutorialFloor", "AS_TutorialAmbient_Placeholder"})

    nav_class = unreal.NavMeshBoundsVolume.static_class()
    nav = unreal.EditorLevelLibrary.spawn_actor_from_class(nav_class, unreal.Vector(0.0, 0.0, 90.0), unreal.Rotator(0.0, 0.0, 0.0))
    set_label(nav, "AS_NavMeshBounds_TutorialFloor", "ActionSquad_TutorialRuntime")
    nav.set_actor_scale3d(unreal.Vector(12.0, 12.0, 2.0))

    if sound_asset:
        ambient_class = unreal.AmbientSound.static_class()
        ambient = unreal.EditorLevelLibrary.spawn_actor_from_class(ambient_class, unreal.Vector(0.0, 0.0, 180.0), unreal.Rotator(0.0, 0.0, 0.0))
        set_label(ambient, "AS_TutorialAmbient_Placeholder", "ActionSquad_TutorialRuntime")
        audio_component = ambient.get_editor_property("audio_component")
        if audio_component:
            audio_component.set_editor_property("sound", sound_asset)

    unreal.EditorLevelLibrary.save_current_level()


def setup_gesture_recorder_level():
    if unreal.EditorAssetLibrary.does_asset_exist(GESTURE_LEVEL):
        unreal.EditorLevelLibrary.load_level(GESTURE_LEVEL)
    else:
        unreal.EditorLevelLibrary.new_level(GESTURE_LEVEL)

    world = unreal.EditorLevelLibrary.get_editor_world()
    world_settings = world.get_world_settings()
    game_mode_class = unreal.load_class(None, "/Script/ActionSquad.GestureRecorderGameMode")
    if game_mode_class:
        world_settings.set_editor_property("default_game_mode", game_mode_class)

    destroy_labeled({
        "AS_GestureRecorderPawn",
        "AS_GestureRecorderBlackFloor",
        "AS_GestureRecorderReferenceScreen",
        "AS_GestureRecorderKeyLight",
        "AS_GestureRecorderSkyLight",
    })

    cube = unreal.load_asset("/Engine/BasicShapes/Cube.Cube")
    black_mat = unreal.load_asset("/Game/Tutorial/Mesh/screen_dark.screen_dark")

    floor = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.StaticMeshActor.static_class(), unreal.Vector(0.0, 0.0, -5.0), unreal.Rotator(0.0, 0.0, 0.0))
    set_label(floor, "AS_GestureRecorderBlackFloor", "ActionSquad_GestureRecorder")
    mesh_comp = floor.get_component_by_class(unreal.StaticMeshComponent)
    if mesh_comp:
        mesh_comp.set_static_mesh(cube)
        mesh_comp.set_relative_scale3d(unreal.Vector(8.0, 8.0, 0.05))
        mesh_comp.set_collision_profile_name("BlockAll")
        if black_mat:
            mesh_comp.set_material(0, black_mat)

    key_light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight.static_class(),
        unreal.Vector(-140.0, -160.0, 250.0),
        unreal.Rotator(-36.0, 35.0, 0.0),
    )
    set_label(key_light, "AS_GestureRecorderKeyLight", "ActionSquad_GestureRecorder")
    key_light_comp = key_light.get_component_by_class(unreal.DirectionalLightComponent)
    if key_light_comp:
        key_light_comp.set_editor_property("intensity", 0.65)
        key_light_comp.set_editor_property("light_color", unreal.Color(70, 170, 255, 255))

    sky_light = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight.static_class(),
        unreal.Vector(0.0, 0.0, 210.0),
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    set_label(sky_light, "AS_GestureRecorderSkyLight", "ActionSquad_GestureRecorder")
    sky_light_comp = sky_light.get_component_by_class(unreal.SkyLightComponent)
    if sky_light_comp:
        sky_light_comp.set_editor_property("intensity", 0.18)

    pawn = unreal.EditorLevelLibrary.spawn_actor_from_class(
        load_bp_class(f"{ASSET_DIR}/BP_GestureRecorderPawn"),
        unreal.Vector(0.0, -80.0, 92.0),
        unreal.Rotator(0.0, 90.0, 0.0),
    )
    set_label(pawn, "AS_GestureRecorderPawn", "ActionSquad_GestureRecorder")
    pawn.set_editor_property("auto_possess_player", unreal.AutoReceiveInput.PLAYER0)

    screen = unreal.EditorLevelLibrary.spawn_actor_from_class(
        load_bp_class(f"{ASSET_DIR}/BP_GestureRecorderScreen"),
        unreal.Vector(130.0, 0.0, 145.0),
        unreal.Rotator(0.0, 180.0, 0.0),
    )
    set_label(screen, "AS_GestureRecorderReferenceScreen", "ActionSquad_GestureRecorder")

    unreal.EditorLevelLibrary.save_current_level()


create_blueprint("BP_GestureRecorderPawn", "/Script/ActionSquad.GestureRecorderPawn")
create_blueprint("BP_GestureRecorderScreen", "/Script/ActionSquad.GestureRecorderScreenActor")
unreal.EditorAssetLibrary.save_directory(ASSET_DIR)
sound = import_audio()
setup_tutorial_navigation_and_audio(sound)
setup_gesture_recorder_level()
unreal.EditorAssetLibrary.save_directory("/Game/Audio")
unreal.EditorAssetLibrary.save_directory(LEVEL_DIR)
unreal.log("Gesture recorder level, tutorial navigation, and placeholder audio are ready.")
