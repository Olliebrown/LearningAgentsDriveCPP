// Fill out your copyright notice in the Description page of Project Settings.


#include "AutonomousCarManagerComponent.h"

UAutonomousCarManagerComponent::UAutonomousCarManagerComponent() {}
UAutonomousCarManagerComponent::~UAutonomousCarManagerComponent() {}

void UAutonomousCarManagerComponent::PostInitProperties()
{
	MaxAgentNum = 32;
	Super::PostInitProperties();
}
