// Copyright Epic Games, Inc. All Rights Reserved.

#include "HiderCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"

#include "Engine/World.h"  // Include World module
#include "DrawDebugHelpers.h"  // Include DrawDebugHelpers for visualization

//////////////////////////////////////////////////////////////////////////
// AHiderCharacter

AHiderCharacter::AHiderCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AHiderCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void AHiderCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AHiderCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AHiderCharacter::Look);

	}

}

void AHiderCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AHiderCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AHiderCharacter::MakeHookEndpointPreviewTransform_Implementation(const float& HitActorLocationZ, const FVector& ImpactPoint, const float& Distance, FTransform& HookEndpointPreviewTransform){

	FVector HookEndpointPreviewLocation;
	HookEndpointPreviewLocation.X = ImpactPoint.X;
	HookEndpointPreviewLocation.Y = ImpactPoint.Y;
	HookEndpointPreviewLocation.Z = HitActorLocationZ + 70;
	HookEndpointPreviewTransform.SetLocation(HookEndpointPreviewLocation);

	FRotator HookEndpointPreviewRotation = UKismetMathLibrary::FindLookAtRotation(HookEndpointPreviewLocation, GetActorLocation());
	HookEndpointPreviewRotation.Roll = 0;
	HookEndpointPreviewRotation.Yaw = HookEndpointPreviewRotation.Yaw + 90;
	HookEndpointPreviewRotation.Pitch = 0;
	HookEndpointPreviewTransform.SetRotation(HookEndpointPreviewRotation.Quaternion());

	FVector HookEndpointPreviewScale = {1, 1, 1};
	HookEndpointPreviewScale = HookEndpointPreviewScale * (Distance / 2000);
	HookEndpointPreviewTransform.SetScale3D(HookEndpointPreviewScale);

}

void AHiderCharacter::MakeRopeTransform_Implementation(const float& HitActorLocationZ, const FVector& ImpactPoint, const float& Distance, FTransform& RopeTransform){

	FVector RopeLocation = GetActorLocation();
	RopeLocation.Z = RopeLocation.Z + 5;
	RopeTransform.SetLocation(RopeLocation);

	FVector RopeLookAtRotationEnd;
	RopeLookAtRotationEnd.X = ImpactPoint.X;
	RopeLookAtRotationEnd.Y = ImpactPoint.Y;
	RopeLookAtRotationEnd.Z = HitActorLocationZ + 115;

	FRotator RopeRotation = UKismetMathLibrary::FindLookAtRotation(RopeLocation, RopeLookAtRotationEnd);
	RopeTransform.SetRotation(RopeRotation.Quaternion());

	FVector RopeScale = {1, 1, 1};
	RopeScale.Z = Distance / 54;
	RopeTransform.SetScale3D(RopeScale);

}

void AHiderCharacter::LineTrace_Implementation(FHitResult& OutHit, FVector_NetQuantize& ImpactPoint, bool& CanSpawnRope)
{
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

	double TraceZ = -1000;

	FVector Start = FollowCamera->GetComponentLocation();
	FVector End = FollowCamera->GetForwardVector() * 4500 + Start;

	FVector NewEnd;
	NewEnd.X = End.X;
	NewEnd.Y = End.Y;

    if (PlayerController)
    {

        while(TraceZ != 1000){

			FCollisionQueryParams TraceParams(FName(TEXT("LineTrace")), true, this);

			NewEnd.Z = End.Z + TraceZ;

        	FHitResult HitResult;
        	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, NewEnd, ECollisionChannel::ECC_MAX, TraceParams);

			//DrawDebugLine(GetWorld(), Start, NewEnd, FColor::Red, true, 1, 0, 1);

			if(bHit){

				OutHit = HitResult; 
				ImpactPoint = HitResult.ImpactPoint;

				AActor* HitActor = HitResult.GetActor();
				UClass* HitActorClass = HitActor->GetClass();
				FString HitActorClassName = HitActorClass->GetName();

				//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, HitActorClassName);

				if(HitActorClassName.Equals("BP_Rope_Edge_C")){

					CanSpawnRope = 1;

					return;

				}

			}

			TraceZ = TraceZ + 50;

		}

    }

	CanSpawnRope = 0;
	return;

}




