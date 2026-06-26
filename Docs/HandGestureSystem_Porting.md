# 手部姿势识别系统 — 移植方案

## 概述

本方案描述如何将 Tiny Trouble VR 项目中的 **Oculus 手部姿势识别系统** 移植到 Blue-Hand-Command VR 项目。

Tiny Trouble 使用官方 Oculus Hand Tools 插件中的 `OculusHandPoseRecognition` 模块，通过 `UHandPoseRecognizer` 组件识别手部姿势，再结合 `BirdFlightGestureComponent` 将姿势映射为游戏命令。

---

## 一、系统架构

```
Oculus XR Plugin (手部追踪数据)
        ↓
UHandPoseRecognizer (左/右手各一个)
   └─ 加载姿势模板 (FHandPose)
   └─ 每帧计算手部角度误差
   └─ 返回置信度最高的姿势
        ↓
CommandGestureComponent (指挥手势组件)
   ├─ 空间模式管理（战斗区 vs 触发区）
   │    ├─ 手距面部 > 30cm → 战斗模式（双拳交互）
   │    └─ 手距面部 ≤ 30cm → 指挥模式（手势识别）
   ├─ 战斗子模块：双拳瞄准 + 交叉射击检测
   ├─ 指挥子模块：7 个手势模板识别
   ├─ 选人逻辑：（1指=A / 2指=B）
   └─ 触发 HUD 反馈
        ↓
AI 队友 Blackboard（接收到命令 → Behavior Tree 执行）
        ↓
玩家看到命令反馈 (蓝色手发光/指令线/队友确认)
```

---

## 二、需要移植的文件

### 2.1 插件层面 — 直接使用，无需移植

Tiny Trouble 的插件位于 `E:\Tiny Trouble VR\TinyTrouble\Plugins\`：

| 插件 | 路径 | 作用 |
|------|------|------|
| **OculusHandTools** | `Plugins/OculusHandTools/` | 包含 `OculusHandPoseRecognition` 模块 |
| **OculusUtils** | `Plugins/OculusUtils/` | 工具模块 |

**做法**：在您的项目 `Plugins/` 目录下直接复制这两个文件夹即可。

### 2.2 关键 C++ 模块依赖

在您的 `BlueHandCommand.Build.cs` 中需要添加：

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Core",
    "CoreUObject",
    "Engine",
    "InputCore",
    "EnhancedInput",
    "HeadMountedDisplay",
    "HandInput",                    // ← Oculus 手部输入
    "OculusHandPoseRecognition",    // ← 手部姿势识别
    "OculusXRInput",                // ← Oculus XR 输入
    "Json",                         // ← 姿势模板文件读写
    "UMG", "Slate", "SlateCore"     // ← 可选，用于 HUD
});
```

### 2.3 C++ 源文件 — 不需要照搬，参考其设计

Tiny Trouble 的 `BirdFlightGestureComponent`（约 60KB）是核心指挥逻辑层。**您不需要直接复制它**，而是参考它的设计模式，为您的项目写一个 `CommandGestureComponent`。

关键参考点：

| Tiny Trouble 的实现 | 您的项目中对应的需求 |
|---------------------|---------------------|
| `UHandPoseRecognizer` (左右手各一个) | 同样需要两个，左右手各一个 |
| `FHandPose` 编码手部姿势 | 完全相同的机制，姿势模板不同 |
| 4 个手势槽位 (TakeoffReady/Launch/SpeedControl/Brake) | 6 个手势 (选队友A/选队友B/移动/警戒/突入/集合) |
| 录制系统 (JSON 姿势模板文件) | 相同的录制/加载机制 |
| `CameraHandInput` (手部骨骼修正) | 同样需要，如果使用 PoseableMesh 显示手 |

关于 `CameraHandInput` 和手部骨骼修正，请参考：
- `E:\Tiny Trouble VR\Doc\HomePawnHandTrackingFixup.md`
- 关键：`HandRootFixupRotation = FQuat(-0.5, -0.5, 0.5, 0.5)`

---

## 三、UHandPoseRecognizer 工作原理

### 3.1 姿势字符串格式

每个姿势被编码为一个字符串，例如"竖食指"（用于选择队友A）：

```
L T0-52-18+51 T1+13-8+30 ...
```

格式说明：
- `L` / `R` — 左手 / 右手
- `T0`~`T3` — 拇指关节
- `I1`~`I3` — 食指关节
- `M1`~`M3` — 中指关节
- `R1`~`R3` — 无名指关节
- `P0`~`P3` — 小指关节
- `W` — 手腕
- 每个关节后的三个数字 = Pitch, Yaw, Roll（角度）
- `+0` 忽略该轴，`*N` 给该关节加权

### 3.2 置信度计算

```
误差 = Σ(所有关节角度差的平方)
置信度 = EAMC / max(EAMC, 实际误差)
  其中 EAMC = Error At Max Confidence
```

### 3.3 录制流程

Tiny Trouble 的做法是在 VR Preview 中直接录制，而 **Blue-Hand-Command 使用专用的纯黑录制关卡**（详情见 [HandPoseRecordingLevel.md](HandPoseRecordingLevel.md)）。

录制流程对比：

| 步骤 | Tiny Trouble | Blue-Hand-Command |
|------|-------------|-------------------|
| 环境 | 游戏场景中录制 | 专用纯黑关卡，零视觉干扰 |
| 倒计时 | 3 秒 | 5 秒 |
| 采集时长 | 1.25 秒 | 5 秒 |
| 手部显示 | 默认材质（依赖场景光） | 自发光材质（纯黑可见） |
| 触发方式 | 键盘 R 键 | 键盘 Space 或指尖点击屏幕 |
| 录制顺序 | 手动选槽位 | 引导式 1→6 依次录制 |

两者底层机制一致：
1. 摆好手势 → 2. 按录制键 → 3. 倒计时 → 4. 采集数据 → 5. 计算平均值和范围 → 6. 写入 JSON 文件

生成的姿势模板保存（我们的项目）：`Saved/HandPoseGestures/BlueHandCommandGestureProfile.json`

---

## 四、完整交互模型（空手方案）

本项目的核心交互原则：**玩家双手始终为空，不拿手柄，不拿武器。**

在这套约束下，如何完成"射击"和"指挥"两种完全不同的操作？答案是**用空间位置区分模式**。

### 4.1 两种操作模式

```
┌─────────────────────────────────────────────────────────┐
│                                                         │
│   战斗模式（手在前方）          指挥模式（手在脸前）       │
│                                                         │
│   🎯 双拳朝前 → 瞄准           ✋ 右手靠近屏幕 → 做手势  │
│   ⚡ 双拳交叉 → 射击           👆 在触发区内比划         │
│   🔄 永远不需要放下手          📋 做完手势回到战斗模式    │
│                                                         │
│   手的空间位置：> 30cm          手的空间位置：< 30cm      │
│   （远离面部）                  （贴近面部）              │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

### 4.2 手势触发区（Gesture Trigger Zone）

这是整个交互模型的核心机制。**在头盔正前方定义一个球状/盒状区域，只有手进入这个区域时，手势识别才生效。**

```
                    ┌──────────┐
                    │  头盔 HMD │
                    └────┬─────┘
                         │
                    ┌────┴─────┐
                    │ 触发区    │
                    │ 半径 20cm │  ← 距面部约 15~30cm
                    │ 球体      │
                    └──────────┘
                         │
                    ══════╪══════ 30cm 分界线
                         │
                    ┌────┴─────┐
                    │ 战斗区    │     ← 距面部 > 30cm
                    │ 手势不生效│
                    └──────────┘
```

**规则：**

| 手的位置 | 手势识别 | 允许的操作 |
|----------|---------|-----------|
| 距面部 > 30cm（战斗区） | ❌ 不识别任何手势 | 双拳瞄准 + 交叉射击 |
| 距面部 ≤ 30cm（触发区） | ✅ 识别命令手势 | 选人 + 5 个命令手势 |

**实现方式：**

```cpp
// 在 CommandGestureComponent 中每帧检测
float HandDistanceFromFace = FVector::Distance(
    RightHandLocation,
    PlayerCamera->GetComponentLocation()
);

if (HandDistanceFromFace <= TriggerRadius)  // 进入触发区
{
    bGestureRecognitionEnabled = true;
    RecognizeCommandGesture();  // 识别当前手势
}
else  // 在战斗区
{
    bGestureRecognitionEnabled = false;
    DetectCombatGesture();  // 检测双拳状态（瞄准 / 射击）
}
```

**为什么这样做：**

```
① 天然防误触 → 手在远处比划，什么都不会发生
② 有意操作 → 把手靠近脸，才能下命令
③ 模式切换自然 → 放下手 = 战斗，抬起手到脸前 = 指挥
④ 和 Tiny Trouble 的手势系统兼容 → 识别逻辑不变，只是加了空间约束
```

### 4.3 射击机制：双拳交叉

玩家在战斗模式下（双手在前方 > 30cm）用双拳瞄准和射击：

```
正常战斗姿态：
  左手✊        右手✊
   (L)          (R)
      ↕            ↕
   ────┴────────────┴────   ← 双拳在同一水平线上
         →→ 瞄准方向 ←←

触发射击：
  左手✊     右手✊
        \   /
         \ /
          ✕               ← 双拳快速交叉/靠近
         / \
        /   \
   ────┴────┴─────────
   
   条件：
   ├── 双拳距离 < 10cm（靠近到几乎相碰）
   ├── 靠近速度 > 阈值（不能慢慢挪过去，要快速合拢）
   └── 双拳在同一水平 ±20cm 范围内
       满足 → 触发射击
```

**射击方向**：两个拳头的中心点指向 + 玩家视线方向 = 子弹落点。

**每次交叉** = 一次射击（单发）或一个短点射（3 发）。不是全自动。

**参数（待调优）：**

| 参数 | 建议初值 | 说明 |
|------|---------|------|
| 触发区半径 | 30cm | 距面部 30cm 以内为手势区 |
| 触发区形状 | 球体（半径 30cm） | 以 HMD 位置为圆心 |
| 双拳触发射击距离 | < 10cm | 两拳之间的距离 |
| 双拳靠近速度阈值 | > 50cm/s | 低于此速度不算"交叉" |
| 每次射击子弹数 | 3 发 | 短点射 |
| 射击后冷却 | 0.3 秒 | 防止连续触发 |

### 4.4 完整的操作流程

```
  空闲状态（手自然下垂）
         │
         ↓
  ┌──────────────────────┐
  │ ① 双拳抬起朝前        │  ← 进入战斗状态
  │    手距面部 > 30cm    │
  │    瞄准 + 双拳交叉射击 │
  └──────────────────────┘
         │
         ↓
  ┌──────────────────────┐
  │ ② 右手靠近面部        │  ← 进入指挥模式
  │    手距面部 ≤ 30cm    │
  │    伸 1 指 → 选中队友A│
  │    伸 2 指 → 选中队友B│
  │    做命令手势 → 执行   │
  └──────────────────────┘
         │
         ↓
  ┌──────────────────────┐
  │ ③ 右手放回前方        │  ← 回到战斗模式
  │    手距面部 > 30cm    │
  │    继续瞄准/射击      │
  └──────────────────────┘
```

### 4.5 7 个手势总结

| # | 手势 | 功能 | 触发条件 |
|---|------|------|---------|
| ① | ☝️ 伸 1 根食指 | **选中队友 A** | 右手在触发区内，食指指天 ≥ 0.5秒 |
| ② | ✌️ 伸 2 根手指 | **选中队友 B** | 右手在触发区内，V 字 ≥ 0.5秒 |
| ③ | 👇 食指指向地面 | **移动到位置** | 已选中队友，指向地面 |
| ④ | ✊ 握拳朝向门 | **破门** | 已选中队友，拳眼对门 |
| ⑤ | ✋ 手掌竖起 | **加强警戒** | 已选中队友，手掌打开 |
| ⑥ | 🤚 手掌前推 | **禁止开火** | 已选中队友，掌推向前 |
| ⑦ | 👊 握拳拉回胸口 | **集合保卫** | 已选中队友，拉回胸前 |

> 手势 ③~⑦ 只在**已选中队友**的状态下生效，未选中时做这些手势无反应。

### 4.6 与 Tiny Trouble 手势系统的兼容性

Tiny Trouble 的 `UHandPoseRecognizer` 和 `FHandPose` 可以无缝支持这套设计：

| Tiny Trouble 机制 | 兼容性 |
|------------------|--------|
| `UHandPoseRecognizer` 手势识别 | ✅ 完全兼容，加载 7 个模板即可 |
| `FHandPose` 姿势编码 | ✅ 完全兼容，编码方式不变 |
| `Confidence` 置信度计算 | ✅ 完全兼容 |
| 录制流程 | ✅ 完全兼容，多录 2 个手势而已 |
| 新增：触发区检测 | 💡 在 `CommandGestureComponent` 中新增 |
| 新增：双拳交叉检测 | 💡 在 `CommandGestureComponent` 中新增 |

---

## 五、移植步骤

### 步骤 1：复制插件
```
复制 E:\Tiny Trouble VR\TinyTrouble\Plugins\OculusHandTools\
复制 E:\Tiny Trouble VR\TinyTrouble\Plugins\OculusUtils\
粘贴到您的项目 Plugins\ 目录
```

### 步骤 2：配置项目文件
```
编辑 .uproject → 添加插件引用 (参见 TinyTrouble.uproject 的 Plugins 段)
编辑 Build.cs → 添加模块依赖
```

### 步骤 3：创建指挥手势组件
参考 `BirdFlightGestureComponent` 的架构，写一个新的 `CommandGestureComponent`：
- 左右手各挂一个 `UHandPoseRecognizer`
- 5 个手势槽位映射到 5 个命令（移动/破门/警戒/禁火/集合）
- 视线选择逻辑：判断玩家视线方向与队友位置的夹角
- 手势确认延迟 0.2~0.4 秒（防误触）

### 步骤 4：录制姿势模板
- 在 VR Preivew 中摆姿势 → 录制
- 调整 EAMC 参数以控制识别灵敏度
- 测试不同光照条件下的稳定性

### 步骤 5：集成到手部显示
如果您想显示蓝色虚拟手（如立项书所述）：
- 参考 `CameraHandInput` 处理手部骨骼旋转
- 使用 `UPoseableMeshComponent` 显示手部网格
- 应用 `HandRootFixupRotation` 修正手腕方向

---

## 六、注意事项

1. **手部追踪版本**：在 `DefaultEngine.ini` 设置 `HandTrackingVersion=Default`（Quest 默认使用最新版本）
2. **追踪频率**：建议 `HandTrackingFrequency=HIGH`（60Hz 而不是 30Hz）
3. **误触防护**：所有命令必须有 0.2~0.4 秒确认时间，并有视觉反馈
4. **备用方案**：如果手势识别不稳定，提供备用的按钮操作方式
5. **姿势模板文件**：需要确保打包时随 APK 一起发布（在 `DefaultEngine.ini` 中 `DirectoriesToAlwaysStageAsNonUFS`）
