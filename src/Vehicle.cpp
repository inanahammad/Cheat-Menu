#include "pch.h"
#include "Vehicle.h"
#include "Menu.h"
#include "Ui.h"
#include "Util.h"
#include <CPopulation.h>
#include <CDamageManager.h>

void Vehicle::FixVehicle(CVehicle *pVeh)
{
#ifdef GTASA
	pVeh->Fix();
#elif GTAVC
	switch (pVeh->m_nVehicleClass)
	{
		case VEHICLE_AUTOMOBILE:
		{
			reinterpret_cast<CAutomobile *>(pVeh)->Fix();
			break;
		}
		case VEHICLE_BIKE:
		{
			reinterpret_cast<CBike *>(pVeh)->Fix();
			break;
		}
	}
#endif	
	pVeh->m_fHealth = 1000.0f;
}

Vehicle::Vehicle()
{
#ifdef GTASA
	ParseVehiclesIDE();
#endif
	ParseCarcolsDAT();

	Events::processScriptsEvent += [this]
	{
#ifdef GTASA
		if (!m_bImagesLoaded)
		{
			Util::LoadTextureDirectory(m_Spawner::m_VehData, PLUGIN_PATH((char*)"CheatMenu\\vehicles.txd"));
			Util::LoadTextureDirectory(m_TuneData, PLUGIN_PATH((char*)"CheatMenu\\components.txd"));
			m_bImagesLoaded = true;
		}
#endif

		uint timer = CTimer::m_snTimeInMilliseconds;
		CPlayerPed* pPlayer = FindPlayerPed();
		CVehicle* pVeh = pPlayer->m_pVehicle;

		if (pPlayer && pVeh)
		{
			int hveh = CPools::GetVehicleRef(pVeh);

#ifdef GTASA
			if (Ui::HotKeyPressed(Menu::m_HotKeys::flipVeh))
			{
				int roll = 0;
				Command<Commands::GET_CAR_ROLL>(hveh, &roll);
				roll += 180;
				Command<Commands::SET_CAR_ROLL>(hveh, roll);
				Command<Commands::SET_CAR_ROLL>(hveh, roll); // z rot fix
			}
#endif

			if (Ui::HotKeyPressed(Menu::m_HotKeys::fixVeh))
			{
				FixVehicle(pVeh);
				SetHelpMessage("Vehicle fixed", false, false, false);
			}

			if (Ui::HotKeyPressed(Menu::m_HotKeys::vehEngine))
			{
				bool state = BY_GAME(!pVeh->m_nVehicleFlags.bEngineBroken, true) || pVeh->m_nVehicleFlags.bEngineOn;

				if (state)
				{
					SetHelpMessage("Vehicle engine off", false, false, false);
				}
				else
				{
					SetHelpMessage("Vehicle engine on", false, false, false);
				}
#ifdef GTASA
				pVeh->m_nVehicleFlags.bEngineBroken = state;
#endif
				pVeh->m_nVehicleFlags.bEngineOn = !state;
			}

			if (Ui::HotKeyPressed(Menu::m_HotKeys::vehInstantStart))
				Command<Commands::SET_CAR_FORWARD_SPEED>(hveh, 40.0f);

			if (Ui::HotKeyPressed(Menu::m_HotKeys::vehInstantStop))
				Command<Commands::SET_CAR_FORWARD_SPEED>(hveh, 0);

			if (m_bNoDamage)
			{
#ifdef GTASA
				pVeh->m_nPhysicalFlags.bBulletProof = true;
				pVeh->m_nPhysicalFlags.bExplosionProof = true;
				pVeh->m_nPhysicalFlags.bFireProof = true;
				pVeh->m_nPhysicalFlags.bCollisionProof = true;
				pVeh->m_nPhysicalFlags.bMeeleProof = true;
				pVeh->m_nVehicleFlags.bCanBeDamaged = true;
#elif GTAVC
				pVeh->m_nFlags.bBulletProof = true;
				pVeh->m_nFlags.bExplosionProof = true;
				pVeh->m_nFlags.bFireProof = true;
				pVeh->m_nFlags.bCollisionProof = true;
				pVeh->m_nFlags.bMeleeProof = true;
#endif
			}

			Command<Commands::SET_CAR_HEAVY>(hveh, m_bVehHeavy);
			Command<Commands::SET_CAR_WATERTIGHT>(hveh, m_bVehWatertight);

			if (m_bLockSpeed)
			{
				Command<Commands::SET_CAR_FORWARD_SPEED>(hveh, m_fLockSpeed);
			}

#ifdef GTASA
			pPlayer->m_nPedFlags.CantBeKnockedOffBike = m_bDontFallBike ? 1 : 2;

			if (m_UnlimitedNitro::m_bEnabled 
			&& BY_GAME(pVeh->m_nVehicleSubClass, pVeh->m_nVehicleClass) == VEHICLE_AUTOMOBILE)
			{
				patch::Set<BYTE>(0x969165, 0, true); // All cars have nitro
				patch::Set<BYTE>(0x96918B, 0, true); // All taxis have nitro

				if (KeyPressed(VK_LBUTTON))
				{
					if (!m_UnlimitedNitro::m_bCompAdded)
					{
						AddComponent("1010", false);
						m_UnlimitedNitro::m_bCompAdded = true;
					}
				}
				else
				{
					if (m_UnlimitedNitro::m_bCompAdded)
					{
						RemoveComponent("1010", false);
						m_UnlimitedNitro::m_bCompAdded = false;
					}
				}
			}

			if (m_Neon::m_bRainbowEffect && timer - m_Neon::m_nRainbowTimer > 50)
			{
				int red, green, blue;

				Util::RainbowValues(red, green, blue, 0.25);
				InstallNeon(pVeh, red, green, blue);
				m_Neon::m_nRainbowTimer = timer;
			}
#endif
		}

#ifdef GTASA
		// Traffic neons
		if (m_Neon::m_bApplyOnTraffic && timer - m_Neon::m_bTrafficTimer > 1000)
		{
			for (CVehicle* veh : CPools::ms_pVehiclePool)
			{
				int chance = 0;

				if (veh->m_nVehicleClass == CLASS_NORMAL) // Normal
					chance = Random(1, 20);

				if (veh->m_nVehicleClass == CLASS_RICHFAMILY) // Rich family
					chance = Random(1, 4);

				if (veh->m_nVehicleClass == CLASS_EXECUTIVE) // Executive
					chance = Random(1, 3);

				if (chance == 1 && !IsNeonInstalled(veh) && veh->m_pDriver != pPlayer)
				{
					InstallNeon(veh, Random(0, 255), Random(0, 255), Random(0, 255));
				}
			}
			m_Neon::m_bTrafficTimer = timer;
		}

		if (m_bBikeFly && pVeh && pVeh->IsDriver(pPlayer))
		{
			if (pVeh->m_nVehicleSubClass == VEHICLE_BIKE || pVeh->m_nVehicleSubClass == VEHICLE_BMX)
			{
				if (sqrt(pVeh->m_vecMoveSpeed.x * pVeh->m_vecMoveSpeed.x
					+ pVeh->m_vecMoveSpeed.y * pVeh->m_vecMoveSpeed.y
					+ pVeh->m_vecMoveSpeed.z * pVeh->m_vecMoveSpeed.z
				) > 0.0
					&& CTimer::ms_fTimeStep > 0.0)
				{
					pVeh->FlyingControl(3, -9999.9902f, -9999.9902f, -9999.9902f, -9999.9902f);
				}
			}
		}
#endif
	};
}

Vehicle::~Vehicle()
{
}

#ifdef GTASA
void Vehicle::AddComponent(const std::string& component, const bool display_message)
{
	try
	{
		CPlayerPed* player = FindPlayerPed();
		int icomp = std::stoi(component);
		int hveh = CPools::GetVehicleRef(player->m_pVehicle);

		CStreaming::RequestModel(icomp, eStreamingFlags::PRIORITY_REQUEST);
		CStreaming::LoadAllRequestedModels(true);
		player->m_pVehicle->AddVehicleUpgrade(icomp);
		CStreaming::SetModelIsDeletable(icomp);

		if (display_message)
			SetHelpMessage("Component added", false, false, false);
	}
	catch (...)
	{
		flog << "Failed to component to vehicle " << component << std::endl;
	}
}


void Vehicle::RemoveComponent(const std::string& component, const bool display_message)
{
	try
	{
		CPlayerPed* player = FindPlayerPed();
		int icomp = std::stoi(component);
		int hveh = CPools::GetVehicleRef(player->m_pVehicle);

		player->m_pVehicle->RemoveVehicleUpgrade(icomp);

		if (display_message)
			SetHelpMessage("Component removed", false, false, false);
	}
	catch (...)
	{
		flog << "Failed to remove component from vehicle " << component << std::endl;
	}
}

// hardcoded for now
int Vehicle::GetRandomTrainIdForModel(int model)
{
	static int train_ids[] = {
		8, 9, // model 449
		0, 3, 6, 10, 12, 13, // model 537
		1, 5, 15 // model 538
	};
	int _start = 0, _end = 0;

	switch (model)
	{
		case 449:
			_start = 0;
			_end = 1;
			break;
		case 537:
			_start = 2;
			_end = 7;
			break;
		case 538:
			_start = 8;
			_end = 10;
			break;
		default:
			SetHelpMessage("Invalid train model", false, false, false);
			return -1;
	}
	int id = Random(_start, _end);
	return train_ids[id];
}

// Get vehicle HandlingId
void Vehicle::ParseVehiclesIDE()
{
	std::string m_FilePath = std::string(paths::GetGameDirPathA()) + "/data/vehicles.ide";

	if (fs::exists(m_FilePath))
	{
		std::ifstream file(m_FilePath);
		std::string line;

		while (getline(file, line))
		{
			if (line[0] <= '0' || line[0] >= '9')
				continue;

			try
			{
				std::string temp;
				std::stringstream ss(line);

				// model
				getline(ss, temp, ',');

				int model = std::stoi(temp);

				// modelname, txd, type, handlingId
				getline(ss, temp, ',');
				getline(ss, temp, ',');
				getline(ss, temp, ',');
				getline(ss, temp, ',');

				temp.erase(std::remove_if(temp.begin(), temp.end(), ::isspace), temp.end());

				m_VehicleIDE[model] = temp;
			}
			catch (...)
			{
				flog << "Error while parsing line, " << line << std::endl;
			}
		}

		file.close();
	}
	else flog << "Vehicle.ide file not found";
}
#endif 

void Vehicle::ParseCarcolsDAT()
{
	std::string m_FilePath = std::string(paths::GetGameDirPathA()) + "/data/carcols.dat";

	if (fs::exists(m_FilePath))
	{
		std::ifstream file(m_FilePath);
		std::string line;

		bool car_section = false;
		bool col_section = false;
		int count = 0;
		while (getline(file, line))
		{
			if (line[0] == '#' || line == "")
				continue;

			if (line[0] == 'c' && line[1] == 'a' && line[2] == 'r')
			{
				car_section = true;
				continue;
			}

			if (line[0] == 'c' && line[1] == 'o' && line[2] == 'l')
			{
				col_section = true;
				continue;
			}

			if (line[0] == 'e' && line[1] == 'n' && line[2] == 'd')
			{
				car_section = false;
				col_section = false;
				continue;
			}

			if (col_section)
			{
				try
				{
					std::string temp;
					std::stringstream ss(line);

					std::replace(temp.begin(), temp.end(), '.', ','); // fix one instance where . is used instead of ,

					// red, green, blue
					getline(ss, temp, ',');
					int red = std::stoi(temp);

					getline(ss, temp, ',');
					int green = std::stoi(temp);

					getline(ss, temp, ',');
					int blue = std::stoi(temp);

					std::vector<float> color = { red / 255.0f, green / 255.0f, blue / 255.0f };
					m_CarcolsColorData.push_back(color);

					++count;
				}
				catch (...)
				{
					flog << "Error while parsing car line, " << line << std::endl;
				}
			}

			if (car_section)
			{
				std::string temp;
				std::stringstream ss(line);

				getline(ss, temp, ',');
				std::string name = temp;
				while (getline(ss, temp, ','))
				{
					try
					{
						std::for_each(name.begin(), name.end(), [](char& c)
						{
							c = ::toupper(c);
						});

						int val = std::stoi(temp);
						if (!(std::find(m_CarcolsCarData[name].begin(), m_CarcolsCarData[name].end(), val) !=
							m_CarcolsCarData[name].end()))
							m_CarcolsCarData[name].push_back(val);
					}
					catch (...)
					{
						flog << "Error while parsing car line, " << line << std::endl;
					}
				}
			}
		}

		file.close();
	}
	else 
	{
		flog << "Error locating Vehicle.ide";
	}
}

void Vehicle::SpawnVehicle(std::string& smodel)
{
#ifdef GTASA
	CPlayerPed* player = FindPlayerPed();
	int hplayer = CPools::GetPedRef(player);

	int imodel = std::stoi(smodel);
	CVehicle* veh = nullptr;

	int interior = BY_GAME(player->m_nAreaCode, player->m_nInterior);

	if (Command<Commands::IS_MODEL_AVAILABLE>(imodel))
	{
		CVector pos = player->GetPosition();
		int speed = 0;

		bool bInVehicle = Command<Commands::IS_CHAR_IN_ANY_CAR>(hplayer);
		if (bInVehicle && m_Spawner::m_bSpawnInside)
		{
			CVehicle* pveh = player->m_pVehicle;
			int hveh = CPools::GetVehicleRef(pveh);
			pos = pveh->GetPosition();

			Command<Commands::GET_CAR_SPEED>(hveh, &speed);

			Command<Commands::WARP_CHAR_FROM_CAR_TO_COORD>(hplayer, pos.x, pos.y, pos.z);

			if (pveh->m_nVehicleClass == VEHICLE_TRAIN)
			{
				Command<Commands::DELETE_MISSION_TRAIN>(hveh);
			}
			else
			{
				Command<Commands::DELETE_CAR>(hveh);
			}
		}

		if (interior == 0)
		{
			if (m_Spawner::m_bSpawnInAir && (CModelInfo::IsHeliModel(imodel) || CModelInfo::IsPlaneModel(imodel)))
			{
				pos.z = 400;
			}
			else
			{
				pos.z -= 5;
			}
		}

		if (CModelInfo::IsTrainModel(imodel))
		{
			int train_id = GetRandomTrainIdForModel(imodel);

			if (train_id == -1) // Unknown train id
				return;

			int hveh = 0;

			// Loading all train related models
			CStreaming::RequestModel(590, PRIORITY_REQUEST);
			CStreaming::RequestModel(538, PRIORITY_REQUEST);
			CStreaming::RequestModel(570, PRIORITY_REQUEST);
			CStreaming::RequestModel(569, PRIORITY_REQUEST);
			CStreaming::RequestModel(537, PRIORITY_REQUEST);
			CStreaming::RequestModel(449, PRIORITY_REQUEST);

			CStreaming::LoadAllRequestedModels(false);

			CTrain* train = nullptr;
			CTrain* carraige = nullptr;
			int track = Random(0, 1);
			int node = CTrain::FindClosestTrackNode(pos, &track);
			CTrain::CreateMissionTrain(pos, (Random(0, 1)) == 1 ? true : false, train_id, &train, &carraige, node,
									   track, false);

			veh = (CVehicle*)train;
			hveh = CPools::GetVehicleRef(veh);
			if (veh->m_pDriver)
				Command<Commands::DELETE_CHAR>(CPools::GetPedRef(veh->m_pDriver));

			if (m_Spawner::m_bSpawnInside)
			{
				Command<Commands::WARP_CHAR_INTO_CAR>(hplayer, hveh);
				Command<Commands::SET_CAR_FORWARD_SPEED>(hveh, speed);
			}
			Command<Commands::MARK_MISSION_TRAIN_AS_NO_LONGER_NEEDED>(hveh);
			Command<Commands::MARK_CAR_AS_NO_LONGER_NEEDED>(hveh);
			CStreaming::SetModelIsDeletable(590);
			CStreaming::SetModelIsDeletable(538);
			CStreaming::SetModelIsDeletable(570);
			CStreaming::SetModelIsDeletable(569);
			CStreaming::SetModelIsDeletable(537);
			CStreaming::SetModelIsDeletable(449);
		}
		else
		{
			CStreaming::RequestModel(imodel, PRIORITY_REQUEST);
			CStreaming::LoadAllRequestedModels(false);

			if (m_Spawner::m_nLicenseText[0] != '\0')
			{
				Command<Commands::CUSTOM_PLATE_FOR_NEXT_CAR>(imodel, m_Spawner::m_nLicenseText);
			}

			int hveh = 0;
			if (m_Spawner::m_bSpawnInside)
			{
				Command<Commands::CREATE_CAR>(imodel, pos.x, pos.y, pos.z + 4.0f, &hveh);
				veh = CPools::GetVehicle(hveh);
				veh->SetHeading(player->GetHeading());
				Command<Commands::WARP_CHAR_INTO_CAR>(hplayer, hveh);
				Command<Commands::SET_CAR_FORWARD_SPEED>(hveh, speed);
			}
			else
			{
				player->TransformFromObjectSpace(pos, CVector(0, 10, 0));
				Command<Commands::CREATE_CAR>(imodel, pos.x, pos.y, pos.z + 3.0f, &hveh);
				veh = CPools::GetVehicle(hveh);
				veh->SetHeading(player->GetHeading() + 55.0f);
			}
			BY_GAME(veh->m_nDoorLock, veh->m_nLockStatus) = CARLOCK_UNLOCKED;
			BY_GAME(veh->m_nAreaCode, veh->m_nInterior) = interior;
			Command<Commands::MARK_CAR_AS_NO_LONGER_NEEDED>(CPools::GetVehicleRef(veh));
			CStreaming::SetModelIsDeletable(imodel);
		}
		veh->m_nVehicleFlags.bHasBeenOwnedByPlayer = true;
	}
#endif
}

std::string Vehicle::GetNameFromModel(int model)
{
	CBaseModelInfo* info = CModelInfo::GetModelInfo(model);

	return (const char*)info + 0x32;
}

int Vehicle::GetModelFromName(const char* name)
{
	int model = 0;
	CBaseModelInfo* pModelInfo = CModelInfo::GetModelInfo((char*)name, &model);

	if (model > 0 && model < 1000000 && GetNameFromModel(model) != "")
	{
		return model;
	}
	else
	{
		return 0;
	}
}

void Vehicle::GenerateHandlingDataFile(int phandling)
{
	FILE* fp = fopen("handling.txt", "w");

	std::string handlingId = m_VehicleIDE[FindPlayerPed()->m_pVehicle->m_nModelIndex];
	float fMass = patch::Get<float>(phandling + 0x4);
	float fTurnMass = patch::Get<float>(phandling + 0xC);
	float fDragMult = patch::Get<float>(phandling + 0x10);
	float CentreOfMassX = patch::Get<float>(phandling + 0x14);
	float CentreOfMassY = patch::Get<float>(phandling + 0x18);
	float CentreOfMassZ = patch::Get<float>(phandling + 0x1C);
	int nPercentSubmerged = patch::Get<int>(phandling + 0x20);
	float fTractionMultiplier = patch::Get<float>(phandling + 0x28);
	float fTractionLoss = patch::Get<float>(phandling + 0xA4);
	float TractionBias = patch::Get<float>(phandling + 0xA8);
	float fEngineAcceleration = patch::Get<float>(phandling + 0x7C) * 12500;
	float fEngineInertia = patch::Get<float>(phandling + 0x80);
	int nDriveType = patch::Get<BYTE>(phandling + 0x74);
	int nEngineType = patch::Get<BYTE>(phandling + 0x75);
	float BrakeDeceleration = patch::Get<float>(phandling + 0x94) * 2500;
	float BrakeBias = patch::Get<float>(phandling + 0x98);
	int ABS = patch::Get<BYTE>(phandling + 0x9C);
	float SteeringLock = patch::Get<float>(phandling + 0xA0);
	float SuspensionForceLevel = patch::Get<float>(phandling + 0xAC);
	float SuspensionDampingLevel = patch::Get<float>(phandling + 0xB0);
	float SuspensionHighSpdComDamp = patch::Get<float>(phandling + 0xB4);
	float Suspension_upper_limit = patch::Get<float>(phandling + 0xB8);
	float Suspension_lower_limit = patch::Get<float>(phandling + 0xBC);
	float Suspension_bias = patch::Get<float>(phandling + 0xC0);
	float Suspension_anti_dive_multiplier = patch::Get<float>(phandling + 0xC4);
	float fCollisionDamageMultiplier = patch::Get<float>(phandling + 0xC8) * 0.338;
	int nMonetaryValue = patch::Get<int>(phandling + 0xD8);

	int MaxVelocity = patch::Get<float>(phandling + 0x84);
	MaxVelocity = MaxVelocity * 206 + (MaxVelocity - 0.918668) * 1501;

	int modelFlags = patch::Get<int>(phandling + 0xCC);
	int handlingFlags = patch::Get<int>(phandling + 0xD0);

	int front_lights = patch::Get<BYTE>(phandling + 0xDC);
	int rear_lights = patch::Get<BYTE>(phandling + 0xDD);
	int vehicle_anim_group = patch::Get<BYTE>(phandling + 0xDE);
	int nNumberOfGears = patch::Get<BYTE>(phandling + 0x76);
	float fSeatOffsetDistance = patch::Get<float>(phandling + 0xD4);

	fprintf(
		fp,
		"\n%s\t%.5g\t%.5g\t%.5g\t%.5g\t%.5g\t%.5g\t%d\t%.5g\t%.5g\t%.5g\t%d\t%d\t%.5g\t%.5g\t%c\t%c\t%.5g\t%.5g\t%d\t%.5g\t%.5g\t%.5g\t%.5g\t%.5g\t%.5g\t%.5g\t%.5g\t%.5g\t%.5g\t%d\t%d\t%d\t%d\t%d\t%d",
		handlingId.c_str(), fMass, fTurnMass, fDragMult, CentreOfMassX, CentreOfMassY, CentreOfMassZ, nPercentSubmerged,
		fTractionMultiplier, fTractionLoss, TractionBias, nNumberOfGears,
		MaxVelocity, fEngineAcceleration, fEngineInertia, nDriveType, nEngineType, BrakeDeceleration, BrakeBias, ABS,
		SteeringLock, SuspensionForceLevel, SuspensionDampingLevel,
		SuspensionHighSpdComDamp, Suspension_upper_limit, Suspension_lower_limit, Suspension_bias,
		Suspension_anti_dive_multiplier, fSeatOffsetDistance,
		fCollisionDamageMultiplier, nMonetaryValue, modelFlags, handlingFlags, front_lights, rear_lights,
		vehicle_anim_group);

	fclose(fp);
}


void Vehicle::Draw()
{
	ImGui::Spacing();
	CPlayerPed* pPlayer = FindPlayerPed();
	int hplayer = CPools::GetPedRef(pPlayer);
	CVehicle *pVeh = pPlayer->m_pVehicle;

	if (ImGui::Button("Blow up cars", ImVec2(Ui::GetSize(BY_GAME(3,2)))))
	{
#ifdef GTASA
		Call<0x439D80>();
#elif GTAVC
		for (CVehicle *pVeh : CPools::ms_pVehiclePool)
		{
			pVeh->BlowUpCar(pPlayer);
		}
#endif
	}

	ImGui::SameLine();

	if (ImGui::Button("Fix vehicle", ImVec2(Ui::GetSize(BY_GAME(3,2)))))
	{
		if (pPlayer && pVeh)
		{
			FixVehicle(pVeh);
		}
	}

#ifdef GTASA
	ImGui::SameLine();

	if (ImGui::Button("Flip vehicle", ImVec2(Ui::GetSize(3))))
	{
		if (pPlayer->m_nPedFlags.bInVehicle)
		{
			int hveh = CPools::GetVehicleRef(pPlayer->m_pVehicle);
			float roll;

			Command<Commands::GET_CAR_ROLL>(hveh, &roll);
			roll += 180;
			Command<Commands::SET_CAR_ROLL>(hveh, roll);
			Command<Commands::SET_CAR_ROLL>(hveh, roll); // z rot fix
		}
	}
#endif

	ImGui::Spacing();

	if (ImGui::BeginTabBar("Vehicle", ImGuiTabBarFlags_NoTooltip + ImGuiTabBarFlags_FittingPolicyScroll))
	{
		CVehicle* pVeh = pPlayer->m_pVehicle;
		bool is_driver = pVeh && pPlayer->m_pVehicle->IsDriver(pPlayer);

		ImGui::Spacing();

		if (ImGui::BeginTabItem("Checkboxes"))
		{
			ImGui::Spacing();
			ImGui::BeginChild("CheckboxesChild");
			ImGui::Columns(2, 0, false);

			Ui::CheckboxAddress("Aggressive drivers", 0x96914F);
			Ui::CheckboxAddress("Aim while driving", 0x969179);
			Ui::CheckboxAddress("All cars have nitro", 0x969165);
			Ui::CheckboxAddress("All taxis have nitro", 0x96918B);
			Ui::CheckboxWithHint("Bikes fly", &m_bBikeFly);
			Ui::CheckboxAddress("Boats fly", 0x969153);
			Ui::CheckboxAddress("Cars fly", 0x969160);
			Ui::CheckboxWithHint("Cars heavy", &m_bVehHeavy);
			if (Ui::CheckboxWithHint("Damage proof", &m_bNoDamage,
				"Every vehicle entered will be damage proof\nBullet, Collision, Explosion, Fire, Meele etc"))
			{
				if (pVeh && !m_bNoDamage)
				{
#ifdef GTASA
				pVeh->m_nPhysicalFlags.bBulletProof = false;
				pVeh->m_nPhysicalFlags.bExplosionProof = false;
				pVeh->m_nPhysicalFlags.bFireProof = false;
				pVeh->m_nPhysicalFlags.bCollisionProof = false;
				pVeh->m_nPhysicalFlags.bMeeleProof = false;
				pVeh->m_nVehicleFlags.bCanBeDamaged = false;
#elif GTAVC
				pVeh->m_nFlags.bBulletProof = false;
				pVeh->m_nFlags.bExplosionProof = false;
				pVeh->m_nFlags.bFireProof = false;
				pVeh->m_nFlags.bCollisionProof = false;
				pVeh->m_nFlags.bMeleeProof = false;
#endif
				}
			}
			Ui::CheckboxAddress("Decreased traffic", 0x96917A);

			ImGui::NextColumn();

			Ui::CheckboxWithHint("Don't fall off bike", &m_bDontFallBike);
			Ui::CheckboxAddress("Drive on water", 0x969152);
			Ui::CheckboxAddressEx("Lock train camera", 0x52A52F, 171, 6);
			Ui::CheckboxAddress("Float away when hit", 0x969166);
			Ui::CheckboxAddress("Green traffic lights", 0x96914E);
			Ui::CheckboxAddress("Perfect handling", 0x96914C);
			Ui::CheckboxAddress("Tank mode", 0x969164);
			Ui::CheckboxWithHint("Unlimited nitro", &m_UnlimitedNitro::m_bEnabled, "Nitro will activate when left clicked\n\
\nEnabling this would disable\nAll cars have nitro\nAll taxis have nitro");
			Ui::CheckboxWithHint("Watertight car", &m_bVehWatertight);
			Ui::CheckboxAddress("Wheels only", 0x96914B);
			ImGui::Columns(1);

			if (is_driver)
			{
				ImGui::NewLine();
				ImGui::TextWrapped("For current vehicle,");

				ImGui::Columns(2, 0, false);

				bool state = false;
#ifdef GTASA
				state = pVeh->m_nVehicleFlags.bAlwaysSkidMarks;
				if (Ui::CheckboxWithHint("Always skid marks", &state, nullptr))
					pVeh->m_nVehicleFlags.bAlwaysSkidMarks = state;
#endif

				state = BY_GAME(pVeh->m_nPhysicalFlags.bBulletProof, pVeh->m_nFlags.bBulletProof);
				if (Ui::CheckboxWithHint("Bullet proof", &state, nullptr, m_bNoDamage))
				{
					BY_GAME(pVeh->m_nPhysicalFlags.bBulletProof, pVeh->m_nFlags.bBulletProof) = state;
				}

				state = BY_GAME(pVeh->m_nPhysicalFlags.bCollisionProof, pVeh->m_nFlags.bCollisionProof);
				if (Ui::CheckboxWithHint("Collision proof", &state, nullptr, m_bNoDamage))
				{
					BY_GAME(pVeh->m_nPhysicalFlags.bCollisionProof, pVeh->m_nFlags.bCollisionProof) = state;
				}

#ifdef GTASA
				state = pVeh->m_nVehicleFlags.bDisableParticles;
				if (Ui::CheckboxWithHint("Disable particles", &state, nullptr))
				{
					pVeh->m_nVehicleFlags.bDisableParticles = state;
				}

				state = pVeh->m_nVehicleFlags.bVehicleCanBeTargetted;
				if (Ui::CheckboxWithHint("Driver targetable", &state, "Driver can be targeted"))
				{
					pVeh->m_nVehicleFlags.bVehicleCanBeTargetted = state;
				}
#endif

				state = BY_GAME(!pVeh->m_nVehicleFlags.bEngineBroken, true) || pVeh->m_nVehicleFlags.bEngineOn;
				if (Ui::CheckboxWithHint("Engine on", &state, nullptr, !is_driver))
				{
#ifdef GTASA
					pVeh->m_nVehicleFlags.bEngineBroken = !state;
#endif
					pVeh->m_nVehicleFlags.bEngineOn = state;
				}
				
				state = BY_GAME(pVeh->m_nPhysicalFlags.bExplosionProof, pVeh->m_nFlags.bExplosionProof);
				if (Ui::CheckboxWithHint("Explosion proof", &state, nullptr, m_bNoDamage))
				{
					BY_GAME(pVeh->m_nPhysicalFlags.bExplosionProof, pVeh->m_nFlags.bExplosionProof) = state;
				}

				state = BY_GAME(pVeh->m_nPhysicalFlags.bFireProof, pVeh->m_nFlags.bFireProof);
				if (Ui::CheckboxWithHint("Fire proof", &state, nullptr, m_bNoDamage))
				{
					BY_GAME(pVeh->m_nPhysicalFlags.bFireProof, pVeh->m_nFlags.bFireProof) = state;
				}

				ImGui::NextColumn();

#ifdef GTASA
				state = pVeh->m_nVehicleFlags.bVehicleCanBeTargettedByHS;
				if (Ui::CheckboxWithHint("HS targetable", &state, "Heat Seaker missile can target this"))
				{
					pVeh->m_nVehicleFlags.bVehicleCanBeTargettedByHS = state;
				}
#endif

				state = !BY_GAME(pVeh->m_bIsVisible, pVeh->m_nFlags.bIsVisible);
				if (Ui::CheckboxWithHint("Invisible car", &state, nullptr, !is_driver))
				{
					BY_GAME(pVeh->m_bIsVisible, pVeh->m_nFlags.bIsVisible) = !state;
				}

				state = BY_GAME(!pVeh->ms_forceVehicleLightsOff, pVeh->m_nVehicleFlags.bLightsOn);
				if (Ui::CheckboxWithHint("Lights on", &state, nullptr, !is_driver))
				{
					BY_GAME(pVeh->ms_forceVehicleLightsOff, pVeh->m_nVehicleFlags.bLightsOn) = state;
				}

				state = BY_GAME(pVeh->m_nDoorLock, pVeh->m_nLockStatus) == CARLOCK_LOCKED_PLAYER_INSIDE;
				if (Ui::CheckboxWithHint("Lock doors", &state, nullptr, !is_driver))
				{
					if (state)
					{
						BY_GAME(pVeh->m_nDoorLock, pVeh->m_nLockStatus) = CARLOCK_LOCKED_PLAYER_INSIDE;
					}
					else
					{
						BY_GAME(pVeh->m_nDoorLock, pVeh->m_nLockStatus) = CARLOCK_UNLOCKED;
					}
				}

				state = BY_GAME(pVeh->m_nPhysicalFlags.bMeeleProof, pVeh->m_nFlags.bMeleeProof);
				if (Ui::CheckboxWithHint("Melee proof", &state, nullptr, m_bNoDamage))
				{
					BY_GAME(pVeh->m_nPhysicalFlags.bMeeleProof, pVeh->m_nFlags.bMeleeProof) = state;
				}

#ifdef GTASA
				state = pVeh->m_nVehicleFlags.bPetrolTankIsWeakPoint;
				if (Ui::CheckboxWithHint("Petrol tank blow", &state, "Vehicle will blow up if petrol tank is shot"))
				{
					pVeh->m_nVehicleFlags.bPetrolTankIsWeakPoint = state;
				}

				state = pVeh->m_nVehicleFlags.bSirenOrAlarm;
				if (Ui::CheckboxWithHint("Siren", &state))
				{
					pVeh->m_nVehicleFlags.bSirenOrAlarm = state;
				}

				state = pVeh->m_nVehicleFlags.bTakeLessDamage;
				if (Ui::CheckboxWithHint("Take less dmg", &state, nullptr))
					pVeh->m_nVehicleFlags.bTakeLessDamage = state;
#endif

				ImGui::Columns(1);
			}

			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Menus"))
		{
			ImGui::Spacing();
			ImGui::BeginChild("MenusChild");
			Ui::EditReference("Density multiplier", CPopulation::PedDensityMultiplier, 0, 1, 10);
			if (ImGui::CollapsingHeader("Enter nearest vehicle as"))
			{
				int hplayer = CPools::GetPedRef(pPlayer);
				CVehicle* pClosestVeh = Util::GetClosestVehicle();

				if (pClosestVeh)
				{
					int seats = pClosestVeh->m_nMaxPassengers;

					ImGui::Spacing();
					ImGui::Columns(2, 0, false);

					ImGui::Text(GetNameFromModel(pClosestVeh->m_nModelIndex).c_str());
					ImGui::NextColumn();
					ImGui::Text("Total seats: %d", (seats + 1));
					ImGui::Columns(1);

					ImGui::Spacing();
					if (ImGui::Button("Driver", ImVec2(Ui::GetSize(2))))
					{
						Command<Commands::WARP_CHAR_INTO_CAR>(hplayer, pClosestVeh);
					}

					for (int i = 0; i < seats; ++i)
					{
						if (i % 2 != 1)
						{
							ImGui::SameLine();
						}

						if (ImGui::Button((std::string("Passenger ") + std::to_string(i + 1)).c_str(),
							ImVec2(Ui::GetSize(2))))
						{
							Command<Commands::WARP_CHAR_INTO_CAR_AS_PASSENGER>(hplayer, pClosestVeh, i);
						}
					}
				}
				else
				{
					ImGui::Text("No nearby vehicles");
				}

				ImGui::Spacing();
				ImGui::Separator();
			}
			if (ImGui::CollapsingHeader("Remove vehicles in radius"))
			{
				ImGui::InputInt("Radius", &m_nVehRemoveRadius);
				ImGui::Spacing();
				if (ImGui::Button("Remove vehicles", Ui::GetSize(1)))
				{
					CPlayerPed* player = FindPlayerPed();
					for (CVehicle* pVeh : CPools::ms_pVehiclePool)
					{
						if (DistanceBetweenPoints(pVeh->GetPosition(), player->GetPosition()) < m_nVehRemoveRadius
							&& !(BY_GAME(player->m_nPedFlags.bInVehicle, true) && player->m_pVehicle == pVeh))
						{
							Command<Commands::DELETE_CAR>(CPools::GetVehicleRef(pVeh));
						}
					}
				}
				ImGui::Spacing();
				ImGui::Separator();
			}
			if (ImGui::CollapsingHeader("Traffic options"))
			{
				static std::vector<Ui::NamedMemory> color{ {"Black", 0x969151}, {"Pink", 0x969150} };
				static std::vector<Ui::NamedMemory> type{
					{"Cheap", 0x96915E}, {"Country", 0x96917B}, {"Fast", 0x96915F}
				};

				Ui::RadioButtonAddress("Color", color);
				ImGui::Spacing();
				Ui::RadioButtonAddress("Type", type);
				ImGui::Spacing();
				ImGui::Separator();
			}

			if (pPlayer && pPlayer->m_pVehicle)
			{
				CVehicle* pVeh = pPlayer->m_pVehicle;
				int hVeh = CPools::GetVehicleRef(pVeh);

				Ui::EditFloat("Dirt level", (int)pVeh + 0x4B0, 0, 7.5, 15);
#ifdef GTASA
				if (pVeh->m_nVehicleClass == VEHICLE_AUTOMOBILE && ImGui::CollapsingHeader("Doors"))
				{
					ImGui::Columns(2, 0, false);
					ImGui::RadioButton("Damage", &m_nDoorMenuButton, 0);
					ImGui::RadioButton("Fix", &m_nDoorMenuButton, 1);
					ImGui::NextColumn();
					ImGui::RadioButton("Open", &m_nDoorMenuButton, 2);
					ImGui::RadioButton("Pop", &m_nDoorMenuButton, 3);
					ImGui::Columns(1);
					ImGui::Spacing();

					int seats = pVeh->m_nMaxPassengers + 1; // passenger + driver
					int doors = seats == 4 ? 6 : 4;
					int hveh = CPools::GetVehicleRef(pVeh);

					if (ImGui::Button("All", ImVec2(Ui::GetSize())))
					{
						for (int i = 0; i < doors; ++i)
						{
							switch (m_nDoorMenuButton)
							{
								case 0:
									Command<Commands::DAMAGE_CAR_DOOR>(hveh, i);
									break;
								case 1:
									Command<Commands::FIX_CAR_DOOR>(hveh, i);
									break;
								case 2:
									Command<Commands::OPEN_CAR_DOOR>(hveh, i);
									break;
								case 3:
									Command<Commands::POP_CAR_DOOR>(hveh, i);
									break;
								default:
									break;
							}
						}
					}

					for (int i = 0; i != doors; ++i)
					{
						if (ImGui::Button(m_DoorNames[i].c_str(), ImVec2(Ui::GetSize(2))))
						{
							switch (m_nDoorMenuButton)
							{
								case 0:
									Command<Commands::DAMAGE_CAR_DOOR>(hveh, i);
									break;
								case 1:
									Command<Commands::FIX_CAR_DOOR>(hveh, i);
									break;
								case 2:
									Command<Commands::OPEN_CAR_DOOR>(hveh, i);
									break;
								case 3:
									Command<Commands::POP_CAR_DOOR>(hveh, i);
									break;
								default:
									break;
							}
						}

						if (i % 2 != 1)
						{
							ImGui::SameLine();
						}
					}

					ImGui::Spacing();
					ImGui::Separator();
				}
#endif
				if (ImGui::CollapsingHeader("Set speed"))
				{
					Ui::CheckboxWithHint("Lock speed", &m_bLockSpeed);
					ImGui::Spacing();
					ImGui::InputFloat("Set", &m_fLockSpeed);
					ImGui::Spacing();

					m_fLockSpeed = m_fLockSpeed > 100 ? 100 : m_fLockSpeed;
					m_fLockSpeed = m_fLockSpeed < 0 ? 0 : m_fLockSpeed;

					if (ImGui::Button("Set speed##brn", ImVec2(Ui::GetSize(2))))
					{
						Command<Commands::SET_CAR_FORWARD_SPEED>(hVeh, m_fLockSpeed);
					}

					ImGui::SameLine();

					if (ImGui::Button("Instant stop##brn", ImVec2(Ui::GetSize(2))))
					{
						Command<Commands::SET_CAR_FORWARD_SPEED>(hVeh, 0);
					}
				}
			}
			ImGui::EndChild();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Spawn"))
		{
			ImGui::Spacing();
			ImGui::Columns(2, 0, false);
			Ui::CheckboxWithHint("Spawn inside", &m_Spawner::m_bSpawnInside, "Spawn inside vehicle as driver");
			ImGui::NextColumn();
			Ui::CheckboxWithHint("Spawn aircraft in air", &m_Spawner::m_bSpawnInAir);
			ImGui::Columns(1);


			ImGui::Spacing();
			ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() - 2.5);
			ImGui::InputTextWithHint("##LicenseText", "License plate text", m_Spawner::m_nLicenseText, 9);

			Ui::DrawImages(m_Spawner::m_VehData.m_ImagesList, ImVec2(100, 75), m_Spawner::m_VehData.m_Categories,
						   m_Spawner::m_VehData.m_Selected, m_Spawner::m_VehData.m_Filter, SpawnVehicle, nullptr,
						   [](std::string str)
						   {
							   return GetNameFromModel(std::stoi(str));
						   });

			ImGui::EndTabItem();
		}
#ifdef GTASA
		if (pPlayer->m_pVehicle && pPlayer->m_nPedFlags.bInVehicle)
		{
			CVehicle* veh = FindPlayerPed()->m_pVehicle;
			int hveh = CPools::GetVehicleRef(veh);
			if (ImGui::BeginTabItem("Color"))
			{
				Paint::UpdateNodeListRecursive(veh);

				ImGui::Spacing();
				if (ImGui::Button("Reset color", ImVec2(Ui::GetSize())))
				{
					Paint::ResetNodeColor(veh, Paint::veh_nodes::selected);
					SetHelpMessage("Color reset", false, false, false);
				}
				ImGui::Spacing();

				Ui::ListBoxStr("Component", Paint::veh_nodes::names_vec, Paint::veh_nodes::selected);

				if (ImGui::ColorEdit3("Color picker", m_Color::m_fColorPicker))
				{
					uchar r = m_Color::m_fColorPicker[0] * 255;
					uchar g = m_Color::m_fColorPicker[1] * 255;
					uchar b = m_Color::m_fColorPicker[2] * 255;
					Paint::SetNodeColor(veh, Paint::veh_nodes::selected, { r, g, b, 255 }, m_Color::m_bMatFilter);
				}

				ImGui::Spacing();
				ImGui::Columns(2, NULL, false);
				ImGui::Checkbox("Material filter", &m_Color::m_bMatFilter);
				ImGui::RadioButton("Primary", &m_Color::m_nRadioButton, 1);
				ImGui::RadioButton("Secondary", &m_Color::m_nRadioButton, 2);
				ImGui::NextColumn();
				ImGui::Checkbox("Show all", &m_Color::bShowAll);
				ImGui::RadioButton("Tertiary", &m_Color::m_nRadioButton, 3);
				ImGui::RadioButton("Quaternary", &m_Color::m_nRadioButton, 4);
				ImGui::Spacing();
				ImGui::Columns(1);
				ImGui::Text("Select color preset:");
				ImGui::Spacing();

				int count = (int)m_CarcolsColorData.size();

				ImVec2 size = Ui::GetSize();
				int btnsInRow = ImGui::GetWindowContentRegionWidth() / (size.y * 2);
				int btnSize = (ImGui::GetWindowContentRegionWidth() - int(ImGuiStyleVar_ItemSpacing) * (btnsInRow -
					0.6 * btnsInRow)) / btnsInRow;

				ImGui::BeginChild("Colorss");

				if (m_Color::bShowAll)
					for (int colorId = 0; colorId < count; ++colorId)
					{
						if (Ui::ColorButton(colorId, m_CarcolsColorData[colorId], ImVec2(btnSize, btnSize)))
							*(uint8_replacement*)(int(veh) + 0x433 + m_Color::m_nRadioButton) = colorId;

						if ((colorId + 1) % btnsInRow != 0)
							ImGui::SameLine(0.0, 4.0);
					}
				else
				{
					std::string vehName = GetNameFromModel(pPlayer->m_pVehicle->m_nModelIndex);
					for (auto entry : m_CarcolsCarData)
					{
						if (entry.first == vehName)
						{
							int count = 1;
							for (int colorId : entry.second)
							{
								if (Ui::ColorButton(colorId, m_CarcolsColorData[colorId],
									ImVec2(btnSize, btnSize)))
									*(uint8_replacement*)(int(veh) + 0x433 + m_Color::m_nRadioButton) = colorId;

								if (count % btnsInRow != 0)
									ImGui::SameLine(0.0, 4.0);
								++count;
							}

							break;
						}
					}
				}

				ImGui::EndChild();
				ImGui::EndTabItem();
			}
			if (Globals::renderer != Render_DirectX11)
			{
				if (ImGui::BeginTabItem("Neons"))
				{
					ImGui::Spacing();
					if (ImGui::Button("Remove neon", ImVec2(Ui::GetSize())))
					{
						RemoveNeon(veh);
						SetHelpMessage("Neon removed", false, false, false);
					}

					ImGui::Spacing();
					ImGui::Columns(2, NULL, false);

					bool pulsing = IsPulsingEnabled(veh);
					if (Ui::CheckboxWithHint("Pulsing neons", &pulsing))
						SetPulsing(veh, pulsing);

					Ui::CheckboxWithHint("Rainbow neons", &m_Neon::m_bRainbowEffect, "Rainbow effect to neon lights");
					ImGui::NextColumn();
					Ui::CheckboxWithHint("Traffic neons", &m_Neon::m_bApplyOnTraffic, "Adds neon lights to traffic vehicles.\n\
	Only some vehicles will have them.");
					ImGui::Columns(1);

					ImGui::Spacing();

					if (ImGui::ColorEdit3("Color picker", m_Neon::m_fColorPicker))
						InstallNeon(veh, m_Neon::m_fColorPicker[0] * 255, m_Neon::m_fColorPicker[1] * 255,
									m_Neon::m_fColorPicker[2] * 255);

					ImGui::Spacing();
					ImGui::Text("Select neon preset:");

					int count = (int)m_CarcolsColorData.size();
					ImVec2 size = Ui::GetSize();
					int btnsInRow = ImGui::GetWindowContentRegionWidth() / (size.y * 2);
					int btnSize = (ImGui::GetWindowContentRegionWidth() - int(ImGuiStyleVar_ItemSpacing) * (btnsInRow -
						0.6 * btnsInRow)) / btnsInRow;

					ImGui::BeginChild("Neonss");

					for (int color_id = 0; color_id < count; ++color_id)
					{
						if (Ui::ColorButton(color_id, m_CarcolsColorData[color_id], ImVec2(btnSize, btnSize)))
						{
							std::vector<float>& color = m_CarcolsColorData[color_id];
							InstallNeon(veh, color[0] * 255, color[1] * 255, color[2] * 255);
						}

						if ((color_id + 1) % btnsInRow != 0)
							ImGui::SameLine(0.0, 4.0);
					}

					ImGui::EndChild();
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Textures"))
				{
					Paint::UpdateNodeListRecursive(veh);

					ImGui::Spacing();
					if (ImGui::Button("Reset texture", ImVec2(Ui::GetSize())))
					{
						Paint::ResetNodeTexture(veh, Paint::veh_nodes::selected);
						SetHelpMessage("Texture reset", false, false, false);
					}
					ImGui::Spacing();

					Ui::ListBoxStr("Component", Paint::veh_nodes::names_vec, Paint::veh_nodes::selected);
					ImGui::Spacing();

					int maxpjob, curpjob;
					Command<Commands::GET_NUM_AVAILABLE_PAINTJOBS>(hveh, &maxpjob);

					if (maxpjob > 0)
					{
						Command<Commands::GET_CURRENT_VEHICLE_PAINTJOB>(hveh, &curpjob);

						if (ImGui::InputInt("Paintjob", &curpjob))
						{
							if (curpjob > maxpjob)
								curpjob = -1;
							if (curpjob < -1)
								curpjob = maxpjob - 1;

							Command<Commands::GIVE_VEHICLE_PAINTJOB>(hveh, curpjob);
						}

						ImGui::Spacing();
					}

					ImGui::Spacing();
					ImGui::SameLine();
					ImGui::Checkbox("Material filter", &m_Color::m_bMatFilter);
					ImGui::Spacing();
					Ui::DrawImages(m_TextureData.m_ImagesList, ImVec2(100, 80), m_TextureData.m_Categories, m_TextureData.m_Selected,
								m_TextureData.m_Filter,
								[](std::string& str)
								{
									Paint::SetNodeTexture(FindPlayerPed()->m_pVehicle, Paint::veh_nodes::selected, str,
															m_Color::m_bMatFilter);
								},
								nullptr,
									[](std::string& str)
								{
									return str;
								});

					ImGui::EndTabItem();
				}
			}
			if (ImGui::BeginTabItem("Tune"))
			{
				ImGui::Spacing();
				Ui::DrawImages(m_TuneData.m_ImagesList, ImVec2(100, 80), m_TuneData.m_Categories, m_TuneData.m_Selected,
							   m_TuneData.m_Filter,
							   [](std::string& str) { AddComponent(str); },
							   [](std::string& str) { RemoveComponent(str); },
							   [](std::string& str) { return str; },
							   [pPlayer](std::string& str)
							   {
								   return ((bool(*)(int, CVehicle*))0x49B010)(std::stoi(str), pPlayer->m_pVehicle);
							   }
				);

				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Handling"))
			{
				ImGui::Spacing();

				CBaseModelInfo* info = CModelInfo::GetModelInfo(pPlayer->m_pVehicle->m_nModelIndex);
				int pHandling = patch::Get<WORD>((int)info + 0x4A, false);
				pHandling *= 0xE0;
				pHandling += 0xC2B9DC;

				if (ImGui::Button("Reset handling", ImVec2(Ui::GetSize(3))))
				{
					gHandlingDataMgr.LoadHandlingData();
					SetHelpMessage("Handling reset", false, false, false);
				}

				ImGui::SameLine();

				if (ImGui::Button("Save to file", ImVec2(Ui::GetSize(3))))
				{
					GenerateHandlingDataFile(pHandling);
					SetHelpMessage("Handling saved", false, false, false);
				}

				ImGui::SameLine();

				if (ImGui::Button("Read more", ImVec2(Ui::GetSize(3))))
					ShellExecute(NULL, "open", "https://projectcerbera.com/gta/sa/tutorials/handling", NULL, NULL,
								 SW_SHOWNORMAL);

				ImGui::Spacing();

				ImGui::BeginChild("HandlingChild");

				static std::vector<Ui::NamedValue> abs{ {"On", 1}, {"Off", 0} };
				Ui::EditRadioButtonAddressEx("Abs", pHandling + 0x9C, abs);

				Ui::EditFloat("Anti dive multiplier", pHandling + 0xC4, 0.0f, 0.0f, 1.0f);
				Ui::EditFloat("Brake bias", pHandling + 0x98, 0.0f, 0.0f, 1.0f);
				Ui::EditFloat("Brake deceleration", pHandling + 0x94, 0.0f, 0.0f, 20.0f, 2500.0f);
				Ui::EditFloat("Centre of mass X", pHandling + 0x14, -10.0f, -10.0f, 10.0f);
				Ui::EditFloat("Centre of mass Y", pHandling + 0x18, -10.0f, -10.0f, 10.0f);
				Ui::EditFloat("Centre of mass Z", pHandling + 0x1C, -10.0f, -10.0f, 10.0f);
				Ui::EditFloat("Collision damage multiplier", pHandling + 0xC8, 0.0f, 0.0f, 1.0f, 0.3381f);
				Ui::EditFloat("Damping level", pHandling + 0xB0, -10.0f, -10.0f, 10.0f); // test later
				Ui::EditFloat("Drag mult", pHandling + 0x10, 0.0f, 0.0f, 30.0f);

				static std::vector<Ui::NamedValue> drive_type{
					{"Front wheel drive", 70}, {"Rear wheel drive", 82}, {"Four wheel drive", 52}
				};
				Ui::EditRadioButtonAddressEx("Drive type", pHandling + 0x74, drive_type);

				Ui::EditFloat("Engine acceleration", pHandling + 0x7C, 0.0f, 0.0f, 49.0f, 12500.0f);
				Ui::EditFloat("Engine inertia", pHandling + 0x80, 0.1f, 0.1f, 400.0f);

				static std::vector<Ui::NamedValue> engine_type{ {"Petrol", 80}, {"Diseal", 68}, {"Electric", 69} };
				Ui::EditRadioButtonAddressEx("Engine type", pHandling + 0x75, engine_type);

				std::vector<Ui::NamedValue> front_lights{ {"Long", 0}, {"Small", 1}, {"Big", 2}, {"Tall", 3} };
				Ui::EditRadioButtonAddressEx("Front lights", pHandling + 0xDC, front_lights);

				Ui::EditFloat("Force level", pHandling + 0xAC, -10.0f, -10.0f, 10.0f); // test later

				Ui::EditBits("Handling flags", pHandling + 0xD0, m_HandlingFlagNames);

				Ui::EditFloat("High speed damping", pHandling + 0xB4, -10.0f, -10.0f, 10.0f); // test later
				Ui::EditFloat("Lower limit", pHandling + 0xBC, -10.0f, -10.0f, 10.0f); // test later
				Ui::EditFloat("Mass", pHandling + 0x4, 1.0f, 1.0f, 50000.0f);

				///fcommon.UpdateAddress({ name = 'Max velocity',address = phandling + 0x84 ,size = 4,min = 0,max = 2,is_float = true,cvalue = 0.01 , save = false })

				Ui::EditBits("Model flags", pHandling + 0xCC, m_ModelFlagNames);

				Ui::EditAddress<int>("Monetary value", pHandling + 0xD8, 1, 1, 100000);
				Ui::EditAddress<BYTE>("Number of gears", pHandling + 0x76, 1, 1, 10);
				Ui::EditAddress<BYTE>("Percent submerged", pHandling + 0x20, 10, 10, 120);

				static std::vector<Ui::NamedValue> rear_lights{ {"Long", 0}, {"Small", 1}, {"Big", 2}, {"Tall", 3} };
				Ui::EditRadioButtonAddressEx("Rear lights", pHandling + 0xDD, rear_lights);

				Ui::EditFloat("Seat offset distance", pHandling + 0xD4, 0.0f, 0.0f, 1.0f);
				Ui::EditFloat("Steering lock", pHandling + 0xA0, 10.0f, 10.0f, 50.0f);
				Ui::EditFloat("Suspension bias", pHandling + 0xC0, 0.0f, 0.0f, 1.0f);
				Ui::EditFloat("Traction bias", pHandling + 0xA8, 0.0f, 0.0f, 1.0f);
				Ui::EditFloat("Traction loss", pHandling + 0xA4, 0.0f, 0.0f, 1.0f);
				Ui::EditFloat("Traction multiplier", pHandling + 0x28, 0.5f, 0.5f, 2.0f);
				Ui::EditFloat("Turn mass", pHandling + 0xC, 20.0f, 20.0f, 1000.0f); // test later
				Ui::EditFloat("Upper limit", pHandling + 0xB8, -1.0f, -1.0f, 1.0f);
				Ui::EditAddress<BYTE>("Vehicle anim group", pHandling + 0xDE, 0, 0, 20);

				ImGui::EndChild();

				ImGui::EndTabItem();
			}
		}
#endif
		ImGui::EndTabBar();
	}
}
