#include "DQM/HcalGeometryVerifier.h"
namespace dqm {

void HcalGeometryVerifier::configure(
    framework::config::Parameters &parameters) {
  hcal_sim_hits_collection_ =
      parameters.getParameter<std::string>("sim_coll_name");
  hcal_rec_hits_collection_ =
      parameters.getParameter<std::string>("rec_coll_name");
  hcal_sim_hits_pass_name_ = parameters.getParameter<std::string>("sim_pass_name");
  hcal_rec_hits_pass_name_ = parameters.getParameter<std::string>("rec_pass_name");
  stop_on_error = parameters.getParameter<bool>("stop_on_error");
  tolerance = parameters.getParameter<double>("tolerance");
}
void HcalGeometryVerifier::analyze(const framework::Event &event) {
  const auto hcal_sim_hits = event.getCollection<ldmx::SimCalorimeterHit>(
      hcal_sim_hits_collection_, hcal_sim_hits_pass_name_);
  const auto hcal_rec_hits = event.getCollection<ldmx::HcalHit>(
      hcal_rec_hits_collection_, hcal_rec_hits_pass_name_);

  for (const auto &hit : hcal_sim_hits) {
    const ldmx::HcalID id{static_cast<unsigned int>(hit.getID())};
    const auto position{hit.getPosition()};
    auto ok{hit_ok(id, {position[0], position[1], position[2]})};
    histograms_.fill("passes_sim", ok);
    switch (id.section()) {
      case ldmx::HcalID::HcalSection::BACK:
        histograms_.fill("passes_sim_back", ok);
        break;
      case ldmx::HcalID::HcalSection::TOP:
        histograms_.fill("passes_sim_top", ok);
        break;
      case ldmx::HcalID::HcalSection::BOTTOM:
        histograms_.fill("passes_sim_bottom", ok);
        break;
      case ldmx::HcalID::HcalSection::LEFT:
        histograms_.fill("passes_sim_left", ok);
        break;
      case ldmx::HcalID::HcalSection::RIGHT:
        histograms_.fill("passes_sim_right", ok);
        break;
    }
  }
  for (const auto &hit : hcal_rec_hits) {
    const ldmx::HcalID id{static_cast<unsigned int>(hit.getID())};
    auto ok{hit_ok(id, {hit.getXPos(), hit.getYPos(), hit.getZPos()})};
    histograms_.fill("passes_rec", ok);
    switch (id.section()) {
      case ldmx::HcalID::HcalSection::BACK:
        histograms_.fill("passes_rec_back", ok);
        break;
      case ldmx::HcalID::HcalSection::TOP:
        histograms_.fill("passes_rec_top", ok);
        break;
      case ldmx::HcalID::HcalSection::BOTTOM:
        histograms_.fill("passes_rec_bottom", ok);
        break;
      case ldmx::HcalID::HcalSection::LEFT:
        histograms_.fill("passes_rec_left", ok);
        break;
      case ldmx::HcalID::HcalSection::RIGHT:
        histograms_.fill("passes_rec_right", ok);
        break;
    }
  }

}  // Analyze
bool HcalGeometryVerifier::hit_ok(const ldmx::HcalID id,
                                  const std::array<double, 3> &position) {
  const auto &geometry = getCondition<ldmx::HcalGeometry>(
      ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);
  auto [index_along, index_across, index_through]{determine_indices(id)};
  const auto center_vec = geometry.getStripCenterPosition(id);
  const std::array<double, 3> center{center_vec.X(), center_vec.Y(),
                                     center_vec.Z()};
  const auto length{geometry.getScintillatorLength(id)};
  bool outside_bounds_along{
      std::abs(position[index_along] - center[index_along]) >
      length / 2 + tolerance};

  const auto width{geometry.getScintillatorWidth()};
  bool outside_bounds_across{
      std::abs(position[index_across] - center[index_across]) >
      width / 2 + tolerance};

  const auto thickness{geometry.getScintillatorThickness()};
  bool outside_bounds_through{
      std::abs(position[index_through] - center[index_through]) >
      thickness / 2 + tolerance};

  if (outside_bounds_along || outside_bounds_across || outside_bounds_through) {
    std::stringstream ss;
    if (tolerance < 1) {
      // Assume tolerance is of form 1e-N
      //
      // Set precision so it will be clear if it is a floating point precision
      // issue or a problem
      ss.precision(-std::log10(tolerance) + 1);
    }
    ss << std::boolalpha;
    double x{position[0]};
    double y{position[1]};
    double z{position[2]};
    ss << id << " has hit position at (" << x << ", " << y << ", " << z
       << ")\nwhich is not within the bounds of the Hcal strip center ("
       << center[0] << ", " << center[1] << ", " << center[2]
       << ") with tolerance " << tolerance << std::endl;
    ss << "Position along the bar: " << position[index_along] << " outside "
       << center[index_along] << " +- " << length / 2 << "? "
       << outside_bounds_along << std::endl;
    ss << "Position across the bar: " << position[index_across] << " outside "
       << center[index_across] << " +- " << width / 2 << "? "
       << outside_bounds_across << std::endl;
    ss << "Position through the bar: " << position[index_through] << " outside "
       << center[index_through] << " +- " << thickness / 2 << "? "
       << outside_bounds_through << std::endl;

    if (stop_on_error) {
      EXCEPTION_RAISE("InvalidPosition", ss.str());
    } else {
      ldmx_log(warn) << ss.str();
    }
    return false;
  }
  return true;
}
std::array<int, 3> HcalGeometryVerifier::determine_indices(
    const ldmx::HcalID id) {
  const auto &geometry = getCondition<ldmx::HcalGeometry>(
      ldmx::HcalGeometry::CONDITIONS_OBJECT_NAME);
  const auto orientation{geometry.getScintillatorOrientation(id)};
  const auto is_lr{id.section() == ldmx::HcalID::HcalSection::LEFT ||
                  id.section() == ldmx::HcalID::HcalSection::RIGHT};

  int index_along{};
  int index_across{};
  int index_through{};
  if (id.section() == ldmx::HcalID::HcalSection::BACK) {
    index_through = 2;  // z
    if (orientation ==
        ldmx::HcalGeometry::ScintillatorOrientation::horizontal) {
      index_along = 0;   // x
      index_across = 1;  // y
    } else {
      index_across = 0;  // x
      index_along = 1;   // y
    }
  } else if (geometry.hasSide3DReadout()) {
    switch (orientation) {
      case ldmx::HcalGeometry::ScintillatorOrientation::horizontal:
        index_across = 2;   // z
        index_along = 0;    // x
        index_through = 1;  // y
        // Horizontal bar in side hcal -> x length, z width, y thick
        break;
      case ldmx::HcalGeometry::ScintillatorOrientation::vertical:
        // Vertical bar in side hcal -> y length, z width, x thick
        index_across = 2;   // z
        index_along = 1;    // y
        index_through = 0;  // x
        break;
      case ldmx::HcalGeometry::ScintillatorOrientation::depth:
        index_along = 2;  // z
        if (is_lr) {
          // Depth bar in side hcal (LR) -> z length, x thick, y width
          index_through = 0;  // x
          index_across = 1;   // y
        } else {
          // Depth bar in side hcal (TB) -> z length, y thick, x width
          index_through = 1;  // y
          index_across = 0;   // x
        }
        break;
    }
  } else {
    // v12 Side hcal
    index_across = 2;  // z
    if (orientation ==
        ldmx::HcalGeometry::ScintillatorOrientation::horizontal) {
      index_along = 0;    // x
      index_through = 1;  // y
    } else {
      index_along = 1;    // y
      index_through = 0;  // x
    }
  }
  return {index_along, index_across, index_through};
}
}  // namespace dqm
DECLARE_ANALYZER(dqm::HcalGeometryVerifier);
