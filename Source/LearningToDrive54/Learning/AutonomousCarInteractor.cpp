// Fill out your copyright notice in the Description page of Project Settings.


#include "AutonomousCarInteractor.h"

#include "LearningAgentsObservations.h"
#include "LearningAgentsActions.h"
#include "LearningAgentsManager.h"

#include "Components/SplineComponent.h"
#include "WheeledVehiclePawn.h"
#include "ChaosWheeledVehicleMovementComponent.h"

#include "ShiftActionEnum.h"
#include "LearningToDrive54/ResetableVehiclePawn.h"

typedef TPair<float, AActor*> FMeasurement;

UAutonomousCarInteractor::UAutonomousCarInteractor() {
	TrackSpline = nullptr;

	TrackDistanceSamples.Add(0.0f);
	TrackDistanceSamples.Add(500.0f);
	TrackDistanceSamples.Add(1000.0f);
	TrackDistanceSamples.Add(1500.0f);
	TrackDistanceSamples.Add(2000.0f);
	TrackDistanceSamples.Add(2500.0f);
	TrackDistanceSamples.Add(3000.0f);
	
	OtherAgentCount = 10;
	bManualTransmission = false;
	MaxTransmissionGear = 5;
}

void UAutonomousCarInteractor::SpecifyAgentObservation_Implementation(
	FLearningAgentsObservationSchemaElement& OutObservationSchemaElement,
	ULearningAgentsObservationSchema* InObservationSchema)
{
	// Define one track observation
	TMap<FName, FLearningAgentsObservationSchemaElement> TrackObservation;
	TrackObservation.Add("Location", ULearningAgentsObservations::SpecifyLocationAlongSplineObservation(
		InObservationSchema, "TrackLocationObservation"));
	TrackObservation.Add("Direction", ULearningAgentsObservations::SpecifyDirectionAlongSplineObservation(
		InObservationSchema, "TrackDirectionObservation"));

	// Wrap in static array and struct
	const auto TrackArrayObservations = ULearningAgentsObservations::SpecifyStaticArrayObservation(
		InObservationSchema,
		ULearningAgentsObservations::SpecifyStructObservation(InObservationSchema, TrackObservation),
		TrackDistanceSamples.Num());

	// Define one agent observation
	TMap<FName, FLearningAgentsObservationSchemaElement> AgentObservation;
	AgentObservation.Add("Location", ULearningAgentsObservations::SpecifyLocationObservation(
		InObservationSchema, "AgentLocationObservation"));
	AgentObservation.Add("Direction", ULearningAgentsObservations::SpecifyDirectionObservation(
		InObservationSchema, "AgentDirectionObservation"));

	// Wrap in static array and struct
	const auto AgentArrayObservations = ULearningAgentsObservations::SpecifyStaticArrayObservation(
		InObservationSchema,
		ULearningAgentsObservations::SpecifyStructObservation(InObservationSchema, AgentObservation),
		OtherAgentCount);

	// Define observation of own speed and RPMs (if manual transmission)
	TMap<FName, FLearningAgentsObservationSchemaElement> SelfObservation;
	SelfObservation.Add("Speed", ULearningAgentsObservations::SpecifyVelocityObservation(InObservationSchema));
	if (bManualTransmission) {
		SelfObservation.Add("RPM", ULearningAgentsObservations::SpecifyFloatObservation(InObservationSchema));
	}

	const auto SelfObservations =
		ULearningAgentsObservations::SpecifyStructObservation(InObservationSchema, SelfObservation);
	
	// Pack everything in a single struct
	TMap<FName, FLearningAgentsObservationSchemaElement> OutObservationMap;
	OutObservationMap.Add("Track", TrackArrayObservations);
	OutObservationMap.Add("Agents", AgentArrayObservations);
	OutObservationMap.Add("Self", SelfObservations);

	// Set to outward bound schema element
	OutObservationSchemaElement = ULearningAgentsObservations::SpecifyStructObservation(InObservationSchema, OutObservationMap);
}

void UAutonomousCarInteractor::GatherAgentObservation_Implementation(
	FLearningAgentsObservationObjectElement& OutObservationObjectElement,
	ULearningAgentsObservationObject* InObservationObject, const int32 AgentId)
{
	// Get reference to agent and it's movement component
	const AResetableVehiclePawn* Agent = Cast<AResetableVehiclePawn>(Manager->GetAgent(AgentId, AResetableVehiclePawn::StaticClass()));
	if (Agent == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Interactor (87): Casting of agent failed."));
		return;
	}

	const UChaosWheeledVehicleMovementComponent* VehicleMovement = Cast<UChaosWheeledVehicleMovementComponent>(Agent->GetVehicleMovementComponent());
	if (VehicleMovement == nullptr) { return; }

	// Make the track observations
	FLearningAgentsObservationObjectElement TrackObservationsArray;
	if (TrackSpline != nullptr) {
		TArray<FLearningAgentsObservationObjectElement> TrackObservations;
		const float Distance = TrackSpline->GetDistanceAlongSplineAtLocation(Agent->GetActorLocation(), ESplineCoordinateSpace::World);
		for (const float Offset : TrackDistanceSamples) {
			// Compute location and distance along track closest to agent
			auto LocationObservation = ULearningAgentsObservations::MakeLocationAlongSplineObservation(
				InObservationObject, TrackSpline, Distance + Offset, Agent->GetActorTransform(),10000.0f,
				"TrackLocationObservation", Agent->IsPlayerControlled(),nullptr, AgentId,
				TrackSpline->GetLocationAtDistanceAlongSpline(Distance + Offset, ESplineCoordinateSpace::World));

			auto DirectionObservation = ULearningAgentsObservations::MakeDirectionAlongSplineObservation(
				InObservationObject, TrackSpline, Distance + Offset, Agent->GetActorTransform(), "TrackDirectionObservation",
				Agent->IsPlayerControlled(),nullptr, AgentId,
				TrackSpline->GetLocationAtDistanceAlongSpline(Distance + Offset, ESplineCoordinateSpace::World));

			// Pack into a map
			TMap<FName, FLearningAgentsObservationObjectElement> TrackObservation;
			TrackObservation.Add("Location", LocationObservation);
			TrackObservation.Add("Direction", DirectionObservation);

			// Add to array
			TrackObservations.Add(ULearningAgentsObservations::MakeStructObservation(InObservationObject, TrackObservation));
		}

		TrackObservationsArray = ULearningAgentsObservations::MakeStaticArrayObservation(InObservationObject, TrackObservations);
	}

	// Prepare to measure distance to other agents
	TArray<FMeasurement> Measurements;
	TArray<UObject*> AllAgents;
	TArray<int32> AllIDs;

	// Loop over other agents and gather distances
	Manager->GetAllAgents(AllAgents, AllIDs, AResetableVehiclePawn::StaticClass());
	for (UObject* AgentObject : AllAgents)
	{
		AActor* OtherAgent = Cast<AActor>(AgentObject);
		if (OtherAgent != nullptr && OtherAgent != Agent)
		{
			float DistToActor = FVector::Distance(Agent->GetActorLocation(), OtherAgent->GetActorLocation());
			Measurements.Add(FMeasurement(DistToActor, OtherAgent));
		}
	}

	// Sort by distance, then pick the closest number
	Algo::SortBy(Measurements, &FMeasurement::Key);
	TArray<FLearningAgentsObservationObjectElement> AgentObservations;
	for (int i = 0; i<OtherAgentCount; i++) {
		// Compute location and distance along track closest to agent
		auto LocationObservation = ULearningAgentsObservations::MakeLocationObservation(
			InObservationObject, Measurements[i].Value->GetActorLocation(), Agent->GetActorTransform(),
			10000.0f, "AgentLocationObservation");

		auto DirectionObservation = ULearningAgentsObservations::MakeDirectionObservation(
			InObservationObject, Measurements[i].Value->GetVelocity(), Agent->GetActorTransform(),
			"AgentDirectionObservation");

		// Pack into a map
		TMap<FName, FLearningAgentsObservationObjectElement> AgentObservation;
		AgentObservation.Add("Location", LocationObservation);
		AgentObservation.Add("Direction", DirectionObservation);

		// Add to array
		AgentObservations.Add(ULearningAgentsObservations::MakeStructObservation(InObservationObject, AgentObservation));
	}

	FLearningAgentsObservationObjectElement AgentObservationsArray =
		ULearningAgentsObservations::MakeStaticArrayObservation(InObservationObject, AgentObservations);
	
	// Make observation of own speed and RPMs (if manual transmission)
	TMap<FName, FLearningAgentsObservationObjectElement> SelfObservations;
	SelfObservations.Add("Speed", ULearningAgentsObservations::MakeVelocityObservation(
		InObservationObject, Agent->GetVelocity(), Agent->GetActorTransform(), 200.0f));
	if (bManualTransmission) {
		SelfObservations.Add("RPM", ULearningAgentsObservations::MakeFloatObservation(
			InObservationObject, VehicleMovement->GetEngineRotationSpeed()));
	}

	// Put observations together into a single struct
	TMap<FName, FLearningAgentsObservationObjectElement> ObservationMap;
	ObservationMap.Add("Track", TrackObservationsArray);
	ObservationMap.Add("Agents", AgentObservationsArray);
	ObservationMap.Add("Self", ULearningAgentsObservations::MakeStructObservation(InObservationObject, SelfObservations));

	// Assign to outward bound observation element
	OutObservationObjectElement = ULearningAgentsObservations::MakeStructObservation(InObservationObject, ObservationMap);
}

void UAutonomousCarInteractor::SpecifyAgentAction_Implementation(
	FLearningAgentsActionSchemaElement& OutActionSchemaElement, ULearningAgentsActionSchema* InActionSchema)
{
	// Enum reference for later use
	static const UEnum* ShiftActionEnumPtr = FindFirstObject<UEnum>(TEXT("EShiftActionEnum"));

	// Build Map of actions
	TMap<FName, FLearningAgentsActionSchemaElement> ActionsMap;
	ActionsMap.Add("Steering", ULearningAgentsActions::SpecifyFloatAction(InActionSchema, "Steering"));
	ActionsMap.Add("ThrottleBrake", ULearningAgentsActions::SpecifyFloatAction(InActionSchema, "ThrottleBrake"));

	// Optionally include shifting for manual transmission
	if (bManualTransmission) {
		TMap<UINT8, float> ShiftProbabilities;
		ShiftProbabilities.Add(ShiftUp, 0.025f);
		ShiftProbabilities.Add(ShiftDown, 0.025f);
		ShiftProbabilities.Add(DoNothing, 0.95f);

		ActionsMap.Add("Gearshift", ULearningAgentsActions::SpecifyEnumAction(
			InActionSchema, ShiftActionEnumPtr, ShiftProbabilities, "Gearshift"));
	}

	// Set to outward bound struct element
	OutActionSchemaElement = ULearningAgentsActions::SpecifyStructAction(InActionSchema, ActionsMap);
}

void UAutonomousCarInteractor::PerformAgentAction_Implementation(const ULearningAgentsActionObject* InActionObject,
	const FLearningAgentsActionObjectElement& InActionObjectElement, const int32 AgentId)
{
	// Enum reference for later use
	static const UEnum* ShiftActionEnumPtr = FindFirstObject<UEnum>(TEXT("EShiftActionEnum"));

	// Get reference to agent and it's movement component
	const AResetableVehiclePawn* Agent = Cast<AResetableVehiclePawn>(Manager->GetAgent(AgentId, AResetableVehiclePawn::StaticClass()));
	if (Agent == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Interactor (176): Casting of agent failed."));
		return;
	}

	UChaosWheeledVehicleMovementComponent* VehicleMovement = Cast<UChaosWheeledVehicleMovementComponent>(Agent->GetVehicleMovementComponent());
	if (VehicleMovement == nullptr) { return; }

	// Extract the main action map
	TMap<FName, FLearningAgentsActionObjectElement> ActionMap;
	if (!ULearningAgentsActions::GetStructAction(ActionMap, InActionObject, InActionObjectElement)) {
		return;
	}

	// Locate individual actions
	const FLearningAgentsActionObjectElement* SteeringAction = ActionMap.Find("Steering");
	const FLearningAgentsActionObjectElement* ThrottleBrakeAction = ActionMap.Find("ThrottleBrake");
	if (SteeringAction == nullptr || ThrottleBrakeAction == nullptr) {
		UE_LOG(LogTemp, Error, TEXT("Missing steering or throttle from action map"));
		return;
	}

	// Retrieve action values
	float SteeringValue = 0.0f, ThrottleBrakeValue = 0.0f;
	if (!ULearningAgentsActions::GetFloatAction(SteeringValue, InActionObject, *SteeringAction, 1.0f, "Steering") ||
		!ULearningAgentsActions::GetFloatAction(ThrottleBrakeValue, InActionObject, *ThrottleBrakeAction, 1.0f, "ThrottleBrake")) {
		UE_LOG(LogTemp, Error, TEXT("Failed to retrieve steering or Thrttle Action value"));
		return;
	}

	// Apply AI input to pawn
	VehicleMovement->SetSteeringInput(SteeringValue);
	if (ThrottleBrakeValue > 0.0f) {
		VehicleMovement->SetThrottleInput(ThrottleBrakeValue);
		VehicleMovement->SetBrakeInput(0.0f);
	} else {
		VehicleMovement->SetThrottleInput(0.0f);
		VehicleMovement->SetBrakeInput(ThrottleBrakeValue);
	}

	// If manual transmission, apply any shifting actions
	if (bManualTransmission) {
		UINT8 GearshiftValue = DoNothing;
		const FLearningAgentsActionObjectElement* GearshiftAction = ActionMap.Find("Gearshift");
		if (GearshiftAction == nullptr) {
			UE_LOG(LogTemp, Error, TEXT("Missing gearshift from action map"));
			return;
		}

		if (!ULearningAgentsActions::GetEnumAction(GearshiftValue, InActionObject, *GearshiftAction, ShiftActionEnumPtr)) {
			UE_LOG(LogTemp, Error, TEXT("Failed to retrieve Gearshift Action value"));
			return;
		}
		
		const int CurrentGear = VehicleMovement->GetTargetGear();
		switch(GearshiftValue) {
			case ShiftUp:
				VehicleMovement->SetTargetGear(FMath::Min(CurrentGear + 1, MaxTransmissionGear), false);
				break;

			case ShiftDown:
				VehicleMovement->SetTargetGear(FMath::Max(CurrentGear - 1, -1), false);
				break;
			
			default:
			case DoNothing:
				break;
		}
	}
}
