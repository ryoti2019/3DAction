#pragma once
#include "Common/Transform.h"
#include "ActorBase.h"

class Collider;

class Tank : public ActorBase
{
public:

	// 回転完了までの時間
	static constexpr float TIME_ROT = 1.0f;

	// コンストラクタ
	Tank(void);

	// デストラクタ
	~Tank(void);

	void Init(void)override;
	void Update(void)override;
	void Draw(void)override;

	// 衝突判定に用いられるコライダ制御
	void AddCollider(std::weak_ptr<Collider> collider);
	void ClearCollider(void);

private:

	// 衝突判定に用いられるコライダ
	std::vector<std::weak_ptr<Collider>> colliders_;

	// ボディの基本情報
	Transform transformBody_;

	// 右車輪の基本情報
	Transform transformRWheel_;

	// 左車輪の基本情報
	Transform transformLWheel_;

	// 砲身の基本情報
	Transform transformBarrel_;

	Quaternion playerRotY_;
	Quaternion goalQuaRot_;

	float stepRotTime_;

	// 回転
	void SetGoalRotate(double rotRad);
	void Rotate(void);

	// 移動操作
	void ProcessMove(void);

	// 発射操作
	void ProcessShot(void);

};

