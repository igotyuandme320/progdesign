#include "boxmanager.h"

BoxManager::BoxManager() = default;

const FlightData &BoxManager::data() const {
    return flightData;
}
