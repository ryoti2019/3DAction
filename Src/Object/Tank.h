#pragma once
#include "Common/Transform.h"
#include "ActorBase.h"

class Tank : public ActorBase
{
public:

	// ��]�����܂ł̎���
	static constexpr float TIME_ROT = 1.0f;

	// �R���X�g���N�^
	Tank(void);

	// �f�X�g���N�^
	~Tank(void);

	void Init(void)override;
	void Update(void)override;
	void Draw(void)override;

private:

	// �{�f�B�̊�{���
	Transform transformBody_;

	// �E�ԗւ̊�{���
	Transform transformRWheel_;

	// ���ԗւ̊�{���
	Transform transformLWheel_;

	// �C�g�̊�{���
	Transform transformBarrel_;

	Quaternion playerRotY_;
	Quaternion goalQuaRot_;

	float stepRotTime_;

	// ��]
	void SetGoalRotate(double rotRad);
	void Rotate(void);

	// �ړ�����
	void ProcessMove(void);

	// ���ˑ���
	void ProcessShot(void);

};

