// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "ActionRPGGame.h"

//#include "GAAttributeComponent.h"
#include "../Items/ARItemInfo.h"
#include "Widgets/GISContainerBaseWidget.h"

#include "GAAttributesBase.h"

//#include "Net/UnrealNetwork.h"
//#include "Engine/ActorChannel.h"

#include "GSPlayerController.h"

#include "ARCharacter.h"

//////////////////////////////////////////////////////////////////////////
// AARCharacter

AARCharacter::AARCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	bReplicates = true;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->NavAgentProps.bCanJump = true;
	GetCharacterMovement()->NavAgentProps.bCanWalk = true;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = ObjectInitializer.CreateDefaultSubobject<USpringArmComponent>(this, TEXT("CameraBoom"));
	CameraBoom->AttachTo(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>(this, TEXT("FollowCamera"));
	FollowCamera->AttachTo(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	Inventory = ObjectInitializer.CreateDefaultSubobject<UGISInventoryBaseComponent>(this, TEXT("Inventory"));
	Inventory->SetIsReplicated(true);
	Inventory->SetNetAddressable();
	//testing out multi commponent interaction.
	
	AbilityBook = ObjectInitializer.CreateDefaultSubobject<UGISInventoryBaseComponent>(this, TEXT("AbilityBook"));
	AbilityBook->SetIsReplicated(true);
	AbilityBook->SetNetAddressable();

	ActionBar = ObjectInitializer.CreateDefaultSubobject<UGISInventoryBaseComponent>(this, TEXT("ActionBar"));
	ActionBar->SetIsReplicated(true);
	ActionBar->SetNetAddressable();
	
	Attributes = ObjectInitializer.CreateDefaultSubobject<UGAAttributeComponent>(this, TEXT("Attributes"));
	Attributes->SetIsReplicated(true);
	Attributes->SetNetAddressable();

	GameEffects = ObjectInitializer.CreateDefaultSubobject<UGESEffectComponent>(this, TEXT("GameEffects"));

	GameEffectManager = ObjectInitializer.CreateDefaultSubobject<UGESEffectManager>(this, TEXT("GameEffectManager"));


	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}
void AARCharacter::BeginPlay()
{
	Super::BeginPlay();
	ActionBar->SetIsReplicated(true);
	ActionBar->SetNetAddressable();

	AbilityBook->SetIsReplicated(true);
	AbilityBook->SetNetAddressable();

	Inventory->SetIsReplicated(true);
	Inventory->SetNetAddressable();

	Attributes->SetIsReplicated(true);
	Attributes->SetNetAddressable();

}

void AARCharacter::CheckIfStructReplicated()
{
	//if (ItemDataTest)
	//{
	//	UGISItemData* testData = ConstructObject<UGISItemData>(ItemDataTest, this);
	//	UGISItemData* lololoTest = ItemDataTestPtr;

	//	UARItemInfo* testItemInfo = Cast<UARItemInfo>(testData);

	//	float lollo = 0;
	//}
}
/** IIGAAttributes Begin */
class UGAAttributesBase* AARCharacter::GetAttributes()
{
	return Attributes->DefaultAttributes;
}

class UGAAttributeComponent* AARCharacter::GetAttributeComponent()
{
	return Attributes;
}
/** IIGAAttributes End */

/** IIGESEffect Begin */
class UGESEffectComponent* AARCharacter::GetEffectComponent()
{
	return GameEffects;
}
class UGESEffectManager* AARCharacter::GetEffectManager()
{
	return GameEffectManager;
}
/** IIGESEffect End */

/** IIGTSocket Begin */
FVector AARCharacter::GetSocketLocation(FName SocketNameIn)
{
	FVector SocketLocation = FVector::ZeroVector;
	if (GetMesh())
		SocketLocation = GetMesh()->GetSocketLocation(SocketNameIn);
	return SocketLocation;
}
/** IIGTSocket End */
//////////////////////////////////////////////////////////////////////////
// Input

void AARCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	// Set up gameplay key bindings
	check(InputComponent);
	InputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	InputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	InputComponent->BindAxis("MoveForward", this, &AARCharacter::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &AARCharacter::MoveRight);

	InputComponent->BindAction("ShowHideInventory", IE_Pressed, this, &AARCharacter::ShowHideInventory);
	InputComponent->BindAction("ShowHideAbilityBook", IE_Pressed, this, &AARCharacter::ShowHideAbilityBook);

	InputComponent->BindAction("ActionButtonTab0Slot0", IE_Pressed, this, &AARCharacter::InputActionBarPressed<0, 0>);
	InputComponent->BindAction("ActionButtonTab0Slot1", IE_Pressed, this, &AARCharacter::InputActionBarPressed<0, 1>);
	InputComponent->BindAction("ActionButtonTab0Slot2", IE_Pressed, this, &AARCharacter::InputActionBarPressed<0, 2>);
	InputComponent->BindAction("ActionButtonTab0Slot3", IE_Pressed, this, &AARCharacter::InputActionBarPressed<0, 3>);
	InputComponent->BindAction("ActionButtonTab0Slot4", IE_Pressed, this, &AARCharacter::InputActionBarPressed<0, 4>);
	InputComponent->BindAction("ActionButtonTab0Slot5", IE_Pressed, this, &AARCharacter::InputActionBarPressed<0, 5>);

	InputComponent->BindAction("ActionButtonTab0Slot0", IE_Released, this, &AARCharacter::InputActionBarReleased<0, 0>);
	InputComponent->BindAction("ActionButtonTab0Slot1", IE_Released, this, &AARCharacter::InputActionBarReleased<0, 1>);
	InputComponent->BindAction("ActionButtonTab0Slot2", IE_Released, this, &AARCharacter::InputActionBarReleased<0, 2>);
	InputComponent->BindAction("ActionButtonTab0Slot3", IE_Released, this, &AARCharacter::InputActionBarReleased<0, 3>);
	InputComponent->BindAction("ActionButtonTab0Slot4", IE_Released, this, &AARCharacter::InputActionBarReleased<0, 4>);
	InputComponent->BindAction("ActionButtonTab0Slot5", IE_Released, this, &AARCharacter::InputActionBarReleased<0, 5>);

	//InputComponent->BindAction("ActionButtonTab0Slot0", IE_Pressed, this, &AARCharacter::InputActionBarPressed<1, 0>);
	//InputComponent->BindAction("ActionButtonTab0Slot1", IE_Pressed, this, &AARCharacter::InputActionBarPressed<1, 1>);
	//InputComponent->BindAction("ActionButtonTab0Slot2", IE_Pressed, this, &AARCharacter::InputActionBarPressed<1, 2>);
	//InputComponent->BindAction("ActionButtonTab0Slot3", IE_Pressed, this, &AARCharacter::InputActionBarPressed<1, 3>);
	//InputComponent->BindAction("ActionButtonTab0Slot4", IE_Pressed, this, &AARCharacter::InputActionBarPressed<1, 4>);
	//InputComponent->BindAction("ActionButtonTab0Slot5", IE_Pressed, this, &AARCharacter::InputActionBarPressed<1, 5>);

	//InputComponent->BindAction("ActionButtonTab0Slot0", IE_Released, this, &AARCharacter::InputActionBarReleased<1, 0>);
	//InputComponent->BindAction("ActionButtonTab0Slot1", IE_Released, this, &AARCharacter::InputActionBarReleased<1, 1>);
	//InputComponent->BindAction("ActionButtonTab0Slot2", IE_Released, this, &AARCharacter::InputActionBarReleased<1, 2>);
	//InputComponent->BindAction("ActionButtonTab0Slot3", IE_Released, this, &AARCharacter::InputActionBarReleased<1, 3>);
	//InputComponent->BindAction("ActionButtonTab0Slot4", IE_Released, this, &AARCharacter::InputActionBarReleased<1, 4>);
	//InputComponent->BindAction("ActionButtonTab0Slot5", IE_Released, this, &AARCharacter::InputActionBarReleased<1, 5>);


	InputComponent->BindAction("SwapActionBars", IE_Pressed, this, &AARCharacter::InputSwapActionBars);
	InputComponent->BindAction("ShowHideEditableActionBars", IE_Pressed, this, &AARCharacter::ShowHideEditableHotbars);
	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	InputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	InputComponent->BindAxis("TurnRate", this, &AARCharacter::TurnAtRate);
	InputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	InputComponent->BindAxis("LookUpRate", this, &AARCharacter::LookUpAtRate);

	// handle touch devices
	InputComponent->BindTouch(IE_Pressed, this, &AARCharacter::TouchStarted);
	InputComponent->BindTouch(IE_Released, this, &AARCharacter::TouchStopped);


	//InputComponent->BindAction("ActionButton1", IE_Pressed, this, &AARCharacter::InputActionBar<0>);
}
void AARCharacter::ActivateAbility()
{

}
void AARCharacter::InputSwapActionBars()
{
	ActionBar->CopyItemsToTargetTabFromLinkedTabs();
}
void AARCharacter::ShowHideEditableHotbars()
{
	ActionBar->ChangeTabVisibility(1);
	ActionBar->ChangeTabVisibility(2);
}
void AARCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	// jump, but only on the first touch
	if (FingerIndex == ETouchIndex::Touch1)
	{
		Jump();
	}
}

void AARCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	if (FingerIndex == ETouchIndex::Touch1)
	{
		StopJumping();
	}
}

void AARCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AARCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AARCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		FRotator Rotation = GetBaseAimRotation();

		FVector Location; //not used, just need for below.
		//Controller->GetPlayerViewPoint(Location, Rotation);
		Rotation.Normalize();
		FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);

	}
}

void AARCharacter::MoveRight(float Value)
{
	if ( (Controller != NULL) && (Value != 0.0f) )
	{
		FRotator Rotation;
		FVector Location; //not used, just need for below.
		Controller->GetPlayerViewPoint(Location, Rotation);
		Rotation.Normalize();
		FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetScaledAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);

	}
}

void AARCharacter::ShowHideInventory()
{
	if (Inventory && Inventory->InventoryContainer)
	{
		//Inventory->ChangeTabVisibility(0);
		//if (Inventory->InventoryContainer->GetVisibility() == ESlateVisibility::Hidden)
		//{
		//	Inventory->InventoryContainer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		//}
		//else if (Inventory->InventoryContainer->GetVisibility() == ESlateVisibility::SelfHitTestInvisible)
		//{
		//	Inventory->InventoryContainer->SetVisibility(ESlateVisibility::Hidden);
		//}
	}
}

void AARCharacter::ShowHideAbilityBook()
{
	if (AbilityBook && AbilityBook->InventoryContainer)
	{
		//AbilityBook->ChangeTabVisibility(0);
		//if (AbilityBook->InventoryContainer->GetVisibility() == ESlateVisibility::Hidden)
		//{
		//	AbilityBook->InventoryContainer->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		//}
		//else if (AbilityBook->InventoryContainer->GetVisibility() == ESlateVisibility::SelfHitTestInvisible)
		//{
		//	AbilityBook->InventoryContainer->SetVisibility(ESlateVisibility::Hidden);
		//}
	}
}