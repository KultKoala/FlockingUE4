//Landon Byrd
//Flocking Implementation

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Flock.generated.h"

class UInstancedStaticMeshComponent;
class UStaticMesh;
class USphereComponent;

//struct for storing values to be passed to flock
USTRUCT()
struct FTInfo {

	GENERATED_BODY()

	uint32 instanceId;
	FVector position;
	FVector velocity;
	FVector acceleration;

};

UCLASS()
class ES_API AFlock : public AActor
{
	GENERATED_BODY()

//user edited values to determine flock behavior

		//user set flock size
		UPROPERTY(EditDefaultsOnly)
		uint32 FlockSize;

		//static mesh to be used for flock
		UPROPERTY(EditDefaultsOnly)
		UStaticMesh * FlockMesh;

		//material to be used for flock
		UPROPERTY(EditDefaultsOnly)
		UMaterialInterface * FlockMaterial;

		//absolute radius member of flock can reach
		UPROPERTY(EditDefaultsOnly)
		float FlockAbsoluteBounds;

		//Weighting factor for cohesion behavior
		UPROPERTY(EditDefaultsOnly, DisplayName = "Cohesion Coefficient")
		float kCoh;

		//Weighting factor for separation behavior
		UPROPERTY(EditDefaultsOnly, DisplayName = "Seperation Coefficient")
		float kSep;

		//Weighting factor for alignment behavior
		UPROPERTY(EditDefaultsOnly, DisplayName = "Alignment Coefficient")
		float kAlign;

		//Radius in which neighboring flock are sought for cohesion
		UPROPERTY(EditDefaultsOnly, DisplayName = "Cohesion Radius")
		float rCohesion;

		//Radius in which neighboring flock are sought for separation
		UPROPERTY(EditDefaultsOnly, DisplayName = "Separation Radius")
		float rSeparation;

		//Radius in which neighboring flock are sought for alignment
		UPROPERTY(EditDefaultsOnly, DisplayName = "Alignment Radius")
		float rAlignment;

		//Maximum acceleration of flock
		UPROPERTY(EditDefaultsOnly, DisplayName = "Maximum Acceleration")
		float maxAccel;

		//Maximum speed of flock
		UPROPERTY(EditDefaultsOnly, DisplayName = "Maximum Velocity")
		float maxVel;

//local data structures used to calculate flock positions
private:
	UInstancedStaticMeshComponent* FlockInstancedMeshComp;

	//back buffer
	TArray<FTInfo> PreviousAgents;

	//front buffer
	TArray<FTInfo> CurrentAgents;

	//Index of array where the current states of each fish are stored
	uint32 currentStatesIndex;

	//Index of array where the previous states of each fish are stored
	uint32 previousStatesIndex;

public:	
	// Sets default values for this actor's properties
	AFlock();

	virtual void PostInitializeComponents() override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Calculate whole flock positions on frame
	void FlockingCalculations(float DTime);

	//Calculate individual flock position
	void FlockMemberCalculations(uint32 FlockIndex, float DTime);

	//Pass calculation to instanced static meshes
	void UpdateFlockMeshes();

};
