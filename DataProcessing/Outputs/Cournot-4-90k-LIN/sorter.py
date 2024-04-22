import os
import shutil

# Function to extract gamma value from DDPG-SimulInfos.txt
def extract_gamma_and_C(file_path):
    gamma = -1
    c = -1
    with open(file_path, 'r') as f:
        for line in f:
            if '--gamma=' in line:
                gamma = float(line.split('=')[1])
            if '--modC=' in line:
                c = float(line.split('=')[1])
    return gamma, c

# Directory containing the simulation files
source_dir = '/Users/aldric-l/Documents/C++ Dev/DDPG-Oligopolies-Simulator/DataProcessing/Outputs/Experimental/E58 (Cournot4)/'
# Directory where sorted folders will be created
target_dir = '/Users/aldric-l/Documents/C++ Dev/DDPG-Oligopolies-Simulator/DataProcessing/Outputs/Experimental/E58 (Cournot4)/'

# Create target directory if it doesn't exist
#if not os.path.exists(target_dir):
    #os.makedirs(target_dir)

# Iterate over each file in the source directory
for file_name in os.listdir(source_dir):
    if file_name.startswith("DDPG-SimulInfos"):
        # Extract gamma value from DDPG-SimulInfos
        gamma, c = extract_gamma_and_C(os.path.join(source_dir, file_name))
        

        # Extract simulation id from DDPG-SimulInfos.txt
        simulation_id = file_name.split('-')[-1].split('.')[0]
            
        # Define folder name based on gamma value
        if c > 0.5:
            folder_name = f"gamma{gamma}Stable"
        else: 
            folder_name = f"gamma{gamma}Unstable"
            #folder_name = f"Gamma_{gamma}"
            
        # Create folder if it doesn't exist
        if not os.path.exists(os.path.join(target_dir, folder_name)):
            os.makedirs(os.path.join(target_dir, folder_name))
            
        # Move files to the correct folder
        for f in ["DDPG-Critics", "DDPG-Cournot-4"]:
            shutil.move(
                os.path.join(source_dir, f"{f}-{simulation_id}.csv"),
                os.path.join(target_dir, folder_name, f"{f}-{simulation_id}.csv")
            )
        shutil.move(
            os.path.join(source_dir, f"DDPG-SimulInfos-{simulation_id}.txt"),
            os.path.join(target_dir, folder_name, f"DDPG-SimulInfos-{simulation_id}.txt")
        )

