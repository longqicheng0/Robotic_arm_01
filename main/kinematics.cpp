#include "kinematics.h"
#include <cmath>
#include <cstddef>

namespace arm{
    constexpr float kPi = 3.14159265358979323846f;
    
    static float degreesToRadians(float degrees){
        return degrees * kPi / 180.0f;
    }

    CartesianPosition forwardKinematics(const JointPositions& jointPositions, const ArmGeometry& geometry)
    {
        //for FK, we will use a simple 3d rotation matrix model to calculate the position of the tool.

        //we would have BPE = BRS*SPE + BtS
        //we would have BPT = BRE*EPT + BtE
        //we would have WPT = WRB*BPT
        //with variables: Theta_Base, Theta_Shoulder, Theta_Elbow, Xse, Zse, Zbs, Lt

        float thetaBaseRad = degreesToRadians(jointPositions.jointDegrees[static_cast<std::size_t>(JointId::Base)]);
        float thetaShoulderRad = degreesToRadians(jointPositions.jointDegrees[static_cast<std::size_t>(JointId::Shoulder)]);
        float thetaElbowRad = degreesToRadians(jointPositions.jointDegrees[static_cast<std::size_t>(JointId::Elbow)]);

        float Xse = geometry.shoulderToElbowXMeters;
        float Zse = geometry.shoulderToElbowZMeters;
        float Zbs = geometry.baseHeightMeters;
        float Lt = geometry.elbowToToolMeters;

        //normally we would write out entire matrix stated in line 17-19
        //but here we expand those by hand down on paper so we save some hassle.

        //by expanding we can obtain radiusMeters is the tool's dist from the bases vertical roation axis
        float radiusMeters = 
            Xse * std::cos(thetaShoulderRad) 
            + Zse * std::sin(thetaShoulderRad) 
            + Lt * std::sin(thetaShoulderRad + thetaElbowRad);

        //z of the toolls can be calculated directly
        float toolZMeters =
            Zbs
            - Xse * std::sin(thetaShoulderRad)
            + Zse * std::cos(thetaShoulderRad)
            + Lt * std::cos(thetaShoulderRad + thetaElbowRad);
        
        //after applying base rotation, we have the final x and y of the tool
        float toolXMeters = radiusMeters * std::cos(thetaBaseRad);
        float toolYMeters = radiusMeters * std::sin(thetaBaseRad);

        return CartesianPosition{
            toolXMeters,
            toolYMeters,
            toolZMeters
        };
    }

}