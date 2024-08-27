#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"

#include "PawnEvents.h"
#include "common/Common.hpp"
#include "common/common_utils/Signal.hpp"

class MultirotorPawnEvents : public PawnEvents
{
public: //types
    typedef msr::airlib::real_T real_T;
    struct RotorActuatorInfo
    {
        real_T rotor_speed = 0;
        int rotor_direction = 0;
        real_T rotor_thrust = 0;
        real_T rotor_control_filtered = 0;
    };

    typedef common_utils::Signal<const std::vector<RotorActuatorInfo>&> ActuatorsSignal;

    struct eVTOLRotorActuatorInfo
    {
        int rotor_mode = 0;
        // rotor_mode = 0 means all rotors are static
        // rotor_mode = 1, lift rotates, cruise static
        // rotor_mode = 2, lift rotates, cruise rotates
        // rotor_mode = 3, lift static, cruise rotates
    };
    typedef common_utils::Signal<const std::vector<eVTOLRotorActuatorInfo>&> eVTOLActuatorsSignal;

public:
    ActuatorsSignal& getActuatorSignal();
    eVTOLActuatorsSignal& geteVTOLActuatorSignal();

private:
    ActuatorsSignal actuator_signal_;
    eVTOLActuatorsSignal evtol_actuator_signal_;
};
