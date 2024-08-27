#include "FlyingPawn.h"
#include "Components/StaticMeshComponent.h"
#include "AirBlueprintLib.h"
#include "common/CommonStructs.hpp"
#include "common/Common.hpp"

AFlyingPawn::AFlyingPawn()
{
    init_id_ = pawn_events_.getActuatorSignal().connect_member(this, &AFlyingPawn::initializeRotors);
    pawn_events_.getActuatorSignal().connect_member(this, &AFlyingPawn::setRotorSpeed);
    init_id_evtol = pawn_events_.geteVTOLActuatorSignal().connect_member(this, &AFlyingPawn::initializeRotors_evtol);
    pawn_events_.geteVTOLActuatorSignal().connect_member(this, &AFlyingPawn::setRotorSpeed_evtol);
}

void AFlyingPawn::BeginPlay()
{
    Super::BeginPlay();
}

void AFlyingPawn::initializeForBeginPlay()
{
    //get references of existing camera
    camera_front_right_ = Cast<APIPCamera>(
        (UAirBlueprintLib::GetActorComponent<UChildActorComponent>(this, TEXT("FrontRightCamera")))->GetChildActor());
    camera_front_left_ = Cast<APIPCamera>(
        (UAirBlueprintLib::GetActorComponent<UChildActorComponent>(this, TEXT("FrontLeftCamera")))->GetChildActor());
    camera_front_center_ = Cast<APIPCamera>(
        (UAirBlueprintLib::GetActorComponent<UChildActorComponent>(this, TEXT("FrontCenterCamera")))->GetChildActor());
    camera_back_center_ = Cast<APIPCamera>(
        (UAirBlueprintLib::GetActorComponent<UChildActorComponent>(this, TEXT("BackCenterCamera")))->GetChildActor());
    camera_bottom_center_ = Cast<APIPCamera>(
        (UAirBlueprintLib::GetActorComponent<UChildActorComponent>(this, TEXT("BottomCenterCamera")))->GetChildActor());
}

void AFlyingPawn::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    pawn_events_.getPawnTickSignal().emit(DeltaSeconds);
}

void AFlyingPawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    camera_front_right_ = nullptr;
    camera_front_left_ = nullptr;
    camera_front_center_ = nullptr;
    camera_back_center_ = nullptr;
    camera_bottom_center_ = nullptr;

    pawn_events_.getActuatorSignal().disconnect_all();
    rotating_movements_.Empty();

    Super::EndPlay(EndPlayReason);
}

const common_utils::UniqueValueMap<std::string, APIPCamera*> AFlyingPawn::getCameras() const
{
    common_utils::UniqueValueMap<std::string, APIPCamera*> cameras;
    cameras.insert_or_assign("front_center", camera_front_center_);
    cameras.insert_or_assign("front_right", camera_front_right_);
    cameras.insert_or_assign("front_left", camera_front_left_);
    cameras.insert_or_assign("bottom_center", camera_bottom_center_);
    cameras.insert_or_assign("back_center", camera_back_center_);

    cameras.insert_or_assign("0", camera_front_center_);
    cameras.insert_or_assign("1", camera_front_right_);
    cameras.insert_or_assign("2", camera_front_left_);
    cameras.insert_or_assign("3", camera_bottom_center_);
    cameras.insert_or_assign("4", camera_back_center_);

    cameras.insert_or_assign("", camera_front_center_);
    cameras.insert_or_assign("fpv", camera_front_center_);

    return cameras;
}

void AFlyingPawn::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation,
                            FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
    pawn_events_.getCollisionSignal().emit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);
}

void AFlyingPawn::setRotorSpeed(const std::vector<MultirotorPawnEvents::RotorActuatorInfo>& rotor_infos)
{
    for (auto rotor_index = 0; rotor_index < rotor_infos.size(); ++rotor_index) {
        auto comp = rotating_movements_[rotor_index];
        if (comp != nullptr) {
            comp->RotationRate.Yaw =
                rotor_infos.at(rotor_index).rotor_speed * rotor_infos.at(rotor_index).rotor_direction *
                180.0f / M_PIf * RotatorFactor;
        }
    }
}

void AFlyingPawn::initializeRotors(const std::vector<MultirotorPawnEvents::RotorActuatorInfo>& rotor_infos)
{
    for (auto i = 0; i < rotor_infos.size(); ++i) {
        rotating_movements_.Add(UAirBlueprintLib::GetActorComponent<URotatingMovementComponent>(this, TEXT("Rotation") + FString::FromInt(i)));
    }
    pawn_events_.getActuatorSignal().disconnect(init_id_);
}
void AFlyingPawn::initializeRotors_evtol(const std::vector<MultirotorPawnEvents::eVTOLRotorActuatorInfo>& rotor_mode_info)
{
    // add rotor of eVTOL
    for (auto i = 0; i < 4; ++i) {
        rotating_movements_evtol.Add(UAirBlueprintLib::GetActorComponent<URotatingMovementComponent>(this, TEXT("RotationL") + FString::FromInt(i)));
        rotating_movements_evtol.Add(UAirBlueprintLib::GetActorComponent<URotatingMovementComponent>(this, TEXT("RotationR") + FString::FromInt(i)));
        rotor_evtol.Add(UAirBlueprintLib::GetActorComponent<UStaticMeshComponent>(this, TEXT("PropL") + FString::FromInt(i)));
        rotor_evtol.Add(UAirBlueprintLib::GetActorComponent<UStaticMeshComponent>(this, TEXT("PropR") + FString::FromInt(i)));
    }
    rotating_movements_evtol.Add(UAirBlueprintLib::GetActorComponent<URotatingMovementComponent>(this, TEXT("RotationC") + FString::FromInt(0)));
    rotor_evtol.Add(UAirBlueprintLib::GetActorComponent<UStaticMeshComponent>(this, TEXT("PropC") + FString::FromInt(0)));
    pawn_events_.geteVTOLActuatorSignal().disconnect(init_id_evtol);
}
void AFlyingPawn::setRotorSpeed_evtol(const std::vector<MultirotorPawnEvents::eVTOLRotorActuatorInfo>& rotor_mode_info)
{
    int rotor_mode = rotor_mode_info[0].rotor_mode;
    if (rotor_mode == 2)
    {
        for (auto i = 0; i < 8; ++i) {
            auto comp = rotating_movements_evtol[i];
            if (comp != nullptr)
            {
                if (i == 0 || i == 3 || i == 5 || i == 6) {
                    comp->RotationRate.Yaw = 9000;
                }
                else {
                    comp->RotationRate.Yaw = -9000;
                }
            }
        }
        auto comp = rotating_movements_evtol[8];
        if (comp != nullptr)
        {

            comp->RotationRate.Yaw = 9000;
        }
    }
    else if (rotor_mode == 0)
    {
        for (auto i = 0; i < 8; ++i) {
            auto comp = rotating_movements_evtol[i];
            if (comp != nullptr)
            {
                comp->RotationRate.Yaw = 0;
            }
            auto prop = rotor_evtol[i];
            if (prop != nullptr)
            {
                FRotator NewRotation(0.0f, 0.0f, 0.0f);
                prop->SetRelativeRotation(NewRotation);
            }
        }
        auto comp = rotating_movements_evtol[8];
        if (comp != nullptr)
        {

            comp->RotationRate.Yaw = 0;
        }
        //auto prop = rotor_evtol[8];
        //if (prop != nullptr)
        //{
        //    FRotator NewRotation(0.0f, -90.0f, 0.0f);
        //    prop->SetRelativeRotation(NewRotation);
        //}
    }
    if (rotor_mode == 1)
    {
        for (auto i = 0; i < 8; ++i) {
            auto comp = rotating_movements_evtol[i];
            if (comp != nullptr)
            {
                if (i == 0 || i == 3 || i == 5 || i == 6) {
                    comp->RotationRate.Yaw = 9000;
                }
                else {
                    comp->RotationRate.Yaw = -9000;
                }
            }
        }
        auto comp = rotating_movements_evtol[8];
        if (comp != nullptr)
        {

            comp->RotationRate.Yaw = 0;
        }
        //auto prop = rotor_evtol[8];
        //if (prop != nullptr)
        //{
        //    FRotator NewRotation(0.0f, 0.0f, 0.0f);
        //    prop->SetRelativeRotation(NewRotation);
        //}
    }
    if (rotor_mode == 3)
    {
        for (auto i = 0; i < 8; ++i) {
            auto comp = rotating_movements_evtol[i];
            if (comp != nullptr)
            {
                comp->RotationRate.Yaw = 0;
            }
            auto prop = rotor_evtol[i];
            if (prop != nullptr)
            {
                FRotator NewRotation(0.0f, 0.0f, 0.0f);
                prop->SetRelativeRotation(NewRotation);
            }
        }
        auto comp = rotating_movements_evtol[8];
        if (comp != nullptr)
        {

            comp->RotationRate.Yaw = 9000;
        }
    }
}