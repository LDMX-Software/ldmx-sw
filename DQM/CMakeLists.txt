cmake_minimum_required(VERSION 3.12)

# Set the project name
project(DQM DESCRIPTION "Data quality management tools."
            LANGUAGES CXX
)

setup_library(module DQM
              dependencies ROOT::Physics
                           Framework::Framework Recon::Event Tools::Tools DetDescr::DetDescr
                           Ecal::Event SimCore::Event SimCore::Bertini Hcal::Event Tracking::Event TrigScint::Event
)

setup_python(package_name LDMX/DQM)
