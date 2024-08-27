#include "MultirotorPawnEvents.h"

MultirotorPawnEvents::ActuatorsSignal& MultirotorPawnEvents::getActuatorSignal()
{
    return actuator_signal_;
}
MultirotorPawnEvents::eVTOLActuatorsSignal& MultirotorPawnEvents::geteVTOLActuatorSignal()
{
    return evtol_actuator_signal_;
}