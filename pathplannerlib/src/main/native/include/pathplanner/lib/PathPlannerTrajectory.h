#pragma once

#include <frc/geometry/Rotation2d.h>
#include <frc/geometry/Translation2d.h>
#include <frc/geometry/Pose2d.h>
#include <frc/trajectory/Trajectory.h>
#include <vector>
#include <units/length.h>
#include <units/velocity.h>
#include <units/acceleration.h>
#include <units/time.h>
#include <units/angular_velocity.h>
#include <units/angular_acceleration.h>
#include <units/area.h>
#include <units/math.h>
#include <units/curvature.h>

namespace pathplanner{
    class PathPlannerTrajectory{
        public:
            class PathPlannerState{
                public:
                    units::second_t time = 0_s;
                    units::meters_per_second_t velocity = 0_mps;
                    units::meters_per_second_squared_t acceleration = 0_mps_sq;
                    frc::Pose2d pose;
                    units::curvature_t curvature{0.0};
                    units::radians_per_second_t angularVel;
                    units::radians_per_second_squared_t angularAccel;
                    frc::Rotation2d holonomicRotation;     
            
                private:
                    units::meter_t position = 0_m;
                    units::meter_t curveRadius = 0_m;
                    units::meter_t deltaPos = 0_m;

                    PathPlannerState interpolate(PathPlannerState endVal, double t);   

                    friend class PathPlannerTrajectory;
            };

            class EventMarker{
                public:
                    std::string name;
                    units::second_t time;
                    frc::Translation2d position;
                
                private:
                    double waypointRelativePos;

                    EventMarker(std::string name, double waypointRelativePos){
                        this->name = name;
                        this->waypointRelativePos = waypointRelativePos;
                    }

                    friend class PathPlannerTrajectory;
                    friend class PathPlanner;
            };
        
        private:
            class Waypoint{
                public:
                    frc::Translation2d anchorPoint;
                    frc::Translation2d prevControl;
                    frc::Translation2d nextControl;
                    units::meters_per_second_t velocityOverride;
                    frc::Rotation2d holonomicRotation;
                    bool isReversal;

                    Waypoint(frc::Translation2d anchorPoint, frc::Translation2d prevControl, frc::Translation2d nextControl, units::meters_per_second_t velocityOverride, frc::Rotation2d holonomicRotation, bool isReversal){
                        this->anchorPoint = anchorPoint;
                        this->prevControl = prevControl;
                        this->nextControl = nextControl;
                        this->velocityOverride = velocityOverride;
                        this->holonomicRotation = holonomicRotation;
                        this->isReversal = isReversal;
                    }
            };

            std::vector<PathPlannerState> states;
            std::vector<EventMarker> markers;

            static std::vector<PathPlannerState> generatePath(std::vector<Waypoint> pathPoints, units::meters_per_second_t maxVel, units::meters_per_second_squared_t maxAccel, bool reversed);
            static std::vector<PathPlannerState> joinSplines(std::vector<Waypoint> pathPoints, units::meters_per_second_t maxVel, double step);
            static void calculateMaxVel(std::vector<PathPlannerState>& states, units::meters_per_second_t maxVel, units::meters_per_second_squared_t maxAccel, bool reversed);
            static void calculateVelocity(std::vector<PathPlannerState>& states, std::vector<Waypoint> pathPoints, units::meters_per_second_squared_t maxAccel);
            static void recalculateValues(std::vector<PathPlannerState>& states, bool reversed);
            static units::meter_t calculateRadius(PathPlannerState s0, PathPlannerState s1, PathPlannerState s2);

            void calculateMarkerTimes(std::vector<Waypoint> pathPoints);

            friend class PathPlanner;
        
        public:
            PathPlannerTrajectory(std::vector<Waypoint> waypoints, std::vector<EventMarker> markers, units::meters_per_second_t maxVelocity, units::meters_per_second_squared_t maxAcceleration, bool reversed);
            PathPlannerTrajectory(std::vector<PathPlannerState> states, std::vector<EventMarker> markers);
            PathPlannerTrajectory(std::vector<PathPlannerState> states);
            PathPlannerTrajectory(){}

            /**
             * @brief Sample the path at a point in time
             * 
             * @param time The time to sample
             * @return The state at the given point in time
             */
            PathPlannerState sample(units::second_t time);

            /**
             * @brief Get all of the states in the path
             * 
             * @return Reference to a vector of all states
             */
            std::vector<PathPlannerState>& getStates() { return this->states; }

            /**
             * @brief Get all of the markers in the path
             * 
             * @return Reference to a vector of all markers
             */
            std::vector<EventMarker>& getMarkers() { return this->markers; }
            
            /**
             * @brief Get the total number of states in the path
             * 
             * @return The number of states
             */
            int numStates() { return getStates().size(); }

            /**
             * @brief Get a state in the path based on its index. In most cases, using sample() is a better method.
             * 
             * @param i The index of the state
             * @return Reference to the state at the given index
             */
            PathPlannerState& getState(int i) { return getStates()[i]; }

            /**
             * @brief Get the initial state of the path
             * 
             * @return Reference to the first state of the path
             */
            PathPlannerState& getInitialState() { return getState(0); }

            /**
             * @brief Get the end state of the path
             * 
             * @return Reference to the last state in the path
             */
            PathPlannerState& getEndState() { return getState(numStates() - 1); }
            
            /**
             * @brief Get the total runtime of the path
             * 
             * @return The path runtime
             */
            units::second_t getTotalTime() { return getEndState().time; }
            
            /**
             * @brief Convert this path to a WPILib compatible trajectory
             * 
             * @return The path as a WPILib trajectory 
             */
            frc::Trajectory asWPILibTrajectory();
    };
}