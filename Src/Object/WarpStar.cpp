#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "../Manager/ResourceManager.h"
#include "../Utility/AsoUtility.h"
#include "Common/Transform.h"
#include "Common/Capsule.h"
#include "Player.h"
#include "WarpStar.h"

WarpStar::WarpStar(Player& player, const Transform& transform) : player_(player) 
{

	transform_ = transform;

	state_ = STATE::NONE;

	// ��ԊǗ�
	stateChanges_.emplace(STATE::IDLE, std::bind(&WarpStar::ChangeStateIdle, this));
	stateChanges_.emplace(STATE::RESERVE, std::bind(&WarpStar::ChangeStateReserve, this));
	stateChanges_.emplace(STATE::MOVE, std::bind(&WarpStar::ChangeStateMove, this));

}

WarpStar::~WarpStar(void)
{
}

void WarpStar::Init(void)
{

	// ���f���̊�{���
	transform_.SetModel(
		resMng_.LoadModelDuplicate(
			ResourceManager::SRC::WARP_STAR)
	);
	transform_.Update();

	// ��]�G�t�F�N�g
	effectRotParticleResId_ = resMng_.Load(
		ResourceManager::SRC::WARP_STAR_ROT_EFF).handleId_;

	// Z������]�̏�Ԃ�ێ����Ă���
	VECTOR angles = transform_.quaRot.ToEuler();
	warpQua_ = Quaternion::Euler(angles.x, angles.y, 0.0f);

	// ���[�v�J�n���W(���[���h���W)
	warpStartPos_ =
		VAdd(transform_.pos, warpQua_.PosAxis(WARP_RELATIVE_POS));

	stepEffect_ = TERM_EFFECT;

	ChangeState(STATE::IDLE);

}

void WarpStar::Update(void)
{

	// �X�V�X�e�b�v
	stateUpdate_();

}

void WarpStar::Draw(void)
{
	MV1DrawModel(transform_.modelId);
}

void WarpStar::ChangeState(STATE state)
{

	// ��ԕύX
	state_ = state;

	// �e��ԑJ�ڂ̏�������
	stateChanges_[state_]();

}

void WarpStar::ChangeStateIdle(void)
{
	stateUpdate_ = std::bind(&WarpStar::UpdateIdle, this);
}

void WarpStar::ChangeStateReserve(void)
{

	stateUpdate_ = std::bind(&WarpStar::UpdateReserve, this);

	// �v���C���[�̏�Ԃ�ύX
	player_.StartWarpReserve(TIME_WARP_RESERVE, warpQua_, warpStartPos_);

}

void WarpStar::ChangeStateMove(void)
{
	stateUpdate_ = std::bind(&WarpStar::UpdateMove, this);
}

void WarpStar::UpdateIdle(void)
{

	// ��]
	RotateZ(SPEED_ROT_IDLE);

	// �G�t�F�N�g
	PlayEffectRotParticle();

	auto player = player_.GetCapsule();
	auto vec = VSub(player.GetCenter(), transform_.pos);
	auto length = AsoUtility::Magnitude(vec);
	if (length < 100.0f)
	{
		ChangeState(STATE::RESERVE);
	}

}

void WarpStar::UpdateReserve(void)
{

	// ��]
	RotateZ(SPEED_ROT_RESERVE);

	// �G�t�F�N�g
	PlayEffectRotParticle();

}

void WarpStar::UpdateMove(void)
{
}

void WarpStar::RotateZ(float speed)
{

	Quaternion axis =
		Quaternion::AngleAxis(
			AsoUtility::Deg2RadF(speed), AsoUtility::AXIS_Z);

	transform_.quaRot = transform_.quaRot.Mult(axis);

	transform_.Update();

}

void WarpStar::PlayEffectRotParticle(void)
{

	// �G�t�F�N�g�Đ�
	effectRotParticlePlayId_ = PlayEffekseer3DEffect(effectRotParticleResId_);

	float SCALE = 5.0f;
	// �傫��
	SetScalePlayingEffekseer3DEffect(effectRotParticlePlayId_, SCALE, SCALE, SCALE);

	// �ʒu
	SyncParticleEffect();

}

void WarpStar::SyncParticleEffect(void)
{

	// �Ǐ]�Ώۂ̈ʒu
	VECTOR followPos = transform_.pos;

	// �Ǐ]�Ώۂ̌���
	Quaternion followRot = transform_.quaRot;

	VECTOR rot = Quaternion::ToEuler(followRot);

	// �Ǐ]�Ώۂ��玩�@�܂ł̑��΍��W
	VECTOR effectPos = followRot.PosAxis(EFFECT_RELATIVE_POS);

	// �G�t�F�N�g�̈ʒu�̍X�V
	effectPos_ = VAdd(followPos, effectPos);

	// �ʒu�̐ݒ�
	SetPosPlayingEffekseer3DEffect(effectRotParticlePlayId_, effectPos_.x, effectPos_.y, effectPos_.z);
	SetRotationPlayingEffekseer3DEffect(effectRotParticlePlayId_, rot.x, rot.y, rot.z);

	transform_.Update();

}
