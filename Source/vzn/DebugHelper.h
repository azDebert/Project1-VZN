#pragma once

namespace Debug
{
	static void Print(const FString& Message, const FColor& Color = FColor::MakeRandomColor(), int32 InKey = -1) // Prints a message to the screen, latest message will be on top
	{
		if (GEngine)
			GEngine->AddOnScreenDebugMessage(InKey, 5.f, Color, Message);
	}

	//UE_LOG(LogTemp, Warning, TEXT("%s"), *Message);
}