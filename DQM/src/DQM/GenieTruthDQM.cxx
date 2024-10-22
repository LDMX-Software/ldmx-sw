//
// Created by Wesley Ketchum on 8/1/24.
//

#include "DQM/GenieTruthDQM.h"

#include <iostream>

#include "GENIE/Framework/Conventions/KineVar.h"
#include "SimCore/Event/HepMC3GenEvent.h"

namespace dqm {

void GenieTruthDQM::configure(framework::config::Parameters& ps) {
  hepmc3CollName_ = ps.getParameter<std::string>("hepmc3CollName");
  hepmc3PassName_ = ps.getParameter<std::string>("hepmc3PassName");
  return;
}

void GenieTruthDQM::onProcessStart() {
  getHistoDirectory();

  ntuple_.create("genie_truth");

  // event info
  ntuple_.addVar<int>("genie_truth", "run");
  ntuple_.addVar<int>("genie_truth", "event");

  ntuple_.addVar<int>("genie_truth", "interaction_type");
  ntuple_.addVar<int>("genie_truth", "scattering_type");
  ntuple_.addVar<int>("genie_truth", "rescatter_code");

  ntuple_.addVar<double>("genie_truth", "x_bj");
  ntuple_.addVar<double>("genie_truth", "y_inel");
  ntuple_.addVar<double>("genie_truth", "Q2");
  ntuple_.addVar<double>("genie_truth", "W");

  ntuple_.addVar<int>("genie_truth", "lep_pdg");

  ntuple_.addVar<double>("genie_truth", "lep_i_px");
  ntuple_.addVar<double>("genie_truth", "lep_i_py");
  ntuple_.addVar<double>("genie_truth", "lep_i_pz");
  ntuple_.addVar<double>("genie_truth", "lep_i_e");

  ntuple_.addVar<double>("genie_truth", "lep_f_px");
  ntuple_.addVar<double>("genie_truth", "lep_f_py");
  ntuple_.addVar<double>("genie_truth", "lep_f_pz");
  ntuple_.addVar<double>("genie_truth", "lep_f_e");

  ntuple_.addVar<int>("genie_truth", "tgt_pdg");
  ntuple_.addVar<double>("genie_truth", "tgt_px");
  ntuple_.addVar<double>("genie_truth", "tgt_py");
  ntuple_.addVar<double>("genie_truth", "tgt_pz");
  ntuple_.addVar<double>("genie_truth", "tgt_e");

  ntuple_.addVar<int>("genie_truth", "hnuc_pdg");
  ntuple_.addVar<double>("genie_truth", "hnuc_px");
  ntuple_.addVar<double>("genie_truth", "hnuc_py");
  ntuple_.addVar<double>("genie_truth", "hnuc_pz");
  ntuple_.addVar<double>("genie_truth", "hnuc_e");

  ntuple_.addVar<int>("genie_truth", "hqrk_pdg");
  ntuple_.addVar<int>("genie_truth", "hqrk_sea");
  ntuple_.addVar<double>("genie_truth", "hadsys_px");
  ntuple_.addVar<double>("genie_truth", "hadsys_py");
  ntuple_.addVar<double>("genie_truth", "hadsys_pz");
  ntuple_.addVar<double>("genie_truth", "hadsys_e");
}

void GenieTruthDQM::onNewRun(const ldmx::RunHeader& runHeader) {
  run_number_ = runHeader.getRunNumber();
}

void GenieTruthDQM::analyze(const framework::Event& event) {
  getHistoDirectory();

  ntuple_.clear();
  ntuple_.setVar<int>("run", run_number_);
  ntuple_.setVar<int>("event", event.getEventNumber());

  auto hepmc3_col = event.getObject<std::vector<ldmx::HepMC3GenEvent> >(
      hepmc3CollName_, hepmc3PassName_);

  if (hepmc3_col.size() < 1) {
    ntuple_.fill();
    return;
  }

  auto const& hepmc3_ev = hepmc3_col.at(0).getHepMCGenEvent();

  // set interaction/scattering codes
  auto interaction_type_ptr = hepmc3_ev.attribute<HepMC3::IntAttribute>(
      "GENIE.Interaction.InteractionType");
  if (interaction_type_ptr)
    ntuple_.setVar<int>("interaction_type", interaction_type_ptr->value());
  auto scattering_type_ptr = hepmc3_ev.attribute<HepMC3::IntAttribute>(
      "GENIE.Interaction.ScatteringType");
  if (scattering_type_ptr)
    ntuple_.setVar<int>("scattering_type", scattering_type_ptr->value());
  auto rescatter_code_ptr =
      hepmc3_ev.attribute<HepMC3::IntAttribute>("GENIE.RescatterCode");
  if (rescatter_code_ptr)
    ntuple_.setVar<int>("rescatter_code", scattering_type_ptr->value());

  // get kinematic vars
  auto kvar_labels_ptr = hepmc3_ev.attribute<HepMC3::VectorIntAttribute>(
      "GENIE.Interaction.KineVarLabels");
  auto kvar_values_ptr = hepmc3_ev.attribute<HepMC3::VectorDoubleAttribute>(
      "GENIE.Interaction.KineVarValues");
  if (kvar_labels_ptr && kvar_values_ptr) {
    auto kvar_labels = kvar_labels_ptr->value();
    auto kvar_values = kvar_values_ptr->value();
    for (size_t i = 0; i < kvar_labels.size(); ++i) {
      if (kvar_labels[i] == genie::EKineVar::kKVSelx)
        ntuple_.setVar<double>("x_bj", kvar_values[i]);
      else if (kvar_labels[i] == genie::EKineVar::kKVSely)
        ntuple_.setVar<double>("y_inel", kvar_values[i]);
      else if (kvar_labels[i] == genie::EKineVar::kKVSelQ2)
        ntuple_.setVar<double>("Q2", kvar_values[i]);
      else if (kvar_labels[i] == genie::EKineVar::kKVSelW)
        ntuple_.setVar<double>("W", kvar_values[i]);
    }
  }

  // electron info
  auto lep_pdg_ptr =
      hepmc3_ev.attribute<HepMC3::IntAttribute>("GENIE.Interaction.ProbePDG");
  if (lep_pdg_ptr) ntuple_.setVar<int>("lep_pdg", lep_pdg_ptr->value());

  auto lep_i_4vec_ptr = hepmc3_ev.attribute<HepMC3::VectorDoubleAttribute>(
      "GENIE.Interaction.ProbeP4");
  if (lep_i_4vec_ptr) {
    auto lep_i_4vec = lep_i_4vec_ptr->value();
    ntuple_.setVar<double>("lep_i_px", lep_i_4vec[0]);
    ntuple_.setVar<double>("lep_i_py", lep_i_4vec[1]);
    ntuple_.setVar<double>("lep_i_pz", lep_i_4vec[2]);
    ntuple_.setVar<double>("lep_i_e", lep_i_4vec[3]);
  }
  auto lep_f_4vec_ptr = hepmc3_ev.attribute<HepMC3::VectorDoubleAttribute>(
      "GENIE.Interaction.FSLeptonP4");
  if (lep_f_4vec_ptr) {
    auto lep_f_4vec = lep_f_4vec_ptr->value();
    ntuple_.setVar<double>("lep_f_px", lep_f_4vec[0]);
    ntuple_.setVar<double>("lep_f_py", lep_f_4vec[1]);
    ntuple_.setVar<double>("lep_f_pz", lep_f_4vec[2]);
    ntuple_.setVar<double>("lep_f_e", lep_f_4vec[3]);
  }

  // target info
  auto tgt_pdg_ptr =
      hepmc3_ev.attribute<HepMC3::IntAttribute>("GENIE.Interaction.TargetPDG");
  if (tgt_pdg_ptr) ntuple_.setVar<int>("tgt_pdg", tgt_pdg_ptr->value());

  auto tgt_4vec_ptr = hepmc3_ev.attribute<HepMC3::VectorDoubleAttribute>(
      "GENIE.Interaction.TargetP4");
  if (tgt_4vec_ptr) {
    auto tgt_4vec = tgt_4vec_ptr->value();
    ntuple_.setVar<double>("tgt_px", tgt_4vec[0]);
    ntuple_.setVar<double>("tgt_py", tgt_4vec[1]);
    ntuple_.setVar<double>("tgt_pz", tgt_4vec[2]);
    ntuple_.setVar<double>("tgt_e", tgt_4vec[3]);
  }

  // hit nucleon info
  auto hnuc_pdg_ptr = hepmc3_ev.attribute<HepMC3::IntAttribute>(
      "GENIE.Interaction.HitNucleonPDG");
  if (hnuc_pdg_ptr) ntuple_.setVar<int>("hnuc_pdg", hnuc_pdg_ptr->value());

  auto hitnuc_4vec_ptr = hepmc3_ev.attribute<HepMC3::VectorDoubleAttribute>(
      "GENIE.Interaction.HitNucleonP4");
  if (hitnuc_4vec_ptr) {
    auto hitnuc_4vec = hitnuc_4vec_ptr->value();
    ntuple_.setVar<double>("hnuc_px", hitnuc_4vec[0]);
    ntuple_.setVar<double>("hnuc_py", hitnuc_4vec[1]);
    ntuple_.setVar<double>("hnuc_pz", hitnuc_4vec[2]);
    ntuple_.setVar<double>("hnuc_e", hitnuc_4vec[3]);
  }
  // hit quark info
  // note: it's only there for some interaction types!
  auto hqrkpdg_ptr = hepmc3_ev.attribute<HepMC3::IntAttribute>(
      "GENIE.Interaction.HitQuarkPDG");
  if (hqrkpdg_ptr) ntuple_.setVar<int>("hqrk_pdg", hqrkpdg_ptr->value());
  auto hqrksea_ptr = hepmc3_ev.attribute<HepMC3::IntAttribute>(
      "GENIE.Interaction.HitSeaQuark");
  if (hqrksea_ptr) ntuple_.setVar<int>("hqrk_sea", hqrkpdg_ptr->value());

  auto hadsys_4vec_ptr = hepmc3_ev.attribute<HepMC3::VectorDoubleAttribute>(
      "GENIE.Interaction.HadSystP4");
  if (hadsys_4vec_ptr) {
    auto hadsys_4vec = hadsys_4vec_ptr->value();
    ntuple_.setVar<double>("hadsys_px", hadsys_4vec[0]);
    ntuple_.setVar<double>("hadsys_py", hadsys_4vec[1]);
    ntuple_.setVar<double>("hadsys_pz", hadsys_4vec[2]);
    ntuple_.setVar<double>("hadsys_e", hadsys_4vec[3]);
  }
  ntuple_.fill();

  return;
}

}  // namespace dqm

DECLARE_ANALYZER_NS(dqm, GenieTruthDQM);
