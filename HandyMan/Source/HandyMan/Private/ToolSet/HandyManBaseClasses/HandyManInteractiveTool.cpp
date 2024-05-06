﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "ToolSet/HandyManBaseClasses/HandyManInteractiveTool.h"

void UHandyManInteractiveTool::Setup()
{
	Super::Setup();
	HandyManAPI = GEditor->GetEditorSubsystem<UHandyManSubsystem>();
}
