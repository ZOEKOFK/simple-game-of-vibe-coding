#pragma once
// ============================================================
// enemy_spawner.h —— 敌人生成器
// 职责：出生位置规则（竞技场边缘/随机点，避免生成在玩家身上）与敌人构造。
// T1 仅声明占位；接口与实现在 T3（构造爬行者）与 T8（三敌种）落地。
// ============================================================
class EnemySpawner {
public:
    // T3：出生位置计算与敌人构造接口在此补充
    //   Vector2 pickSpawnPos(const Vector2& playerPos, float safetyRadius);
    //   std::unique_ptr<Enemy> spawn(const std::string& typeId, int wave);
};
