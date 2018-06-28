////////////////////////////////////////////////////////////////////////
/// \file  FilterCryostatNus_module.cc
/// \brief EDFilter to require projected generator trajectories in volumes within a particular time window.
///
/// \author  Matthew.Bass@physics.ox.ac.uk
////////////////////////////////////////////////////////////////////////
#ifndef FILTER_FilterCryostatNus_H
#define FILTER_FilterCryostatNus_H

/// Framework includes
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/EDFilter.h"

// Framework includes
#include "art/Framework/Principal/Event.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Principal/Handle.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "cetlib_except/exception.h"

// LArSoft Includes
#include "nusimdata/SimulationBase/MCTruth.h"
#include "larcore/Geometry/Geometry.h"
//#include "larcore/Geometry/geo_vectors_utils.h" // geo::vect namespace
#include "larcorealg/Geometry/GeometryCore.h"

// C++ Includes
#include <cmath> // std::abs()
#include <vector>


namespace simfilter {

  class FilterCryostatNus : public art::EDFilter
  {
  public:

    explicit FilterCryostatNus(fhicl::ParameterSet const &pset);

    virtual bool filter(art::Event&) override;
    //virtual void reconfigure(fhicl::ParameterSet const&)  ;

  private:

    bool   fKeepNusInCryostat; // true: keep cryostat nuint; false: filter them
  };

} // namespace simfilter

namespace simfilter {

  FilterCryostatNus::FilterCryostatNus(fhicl::ParameterSet const& pset) :
    fKeepNusInCryostat    (pset.get<bool>("KeepNusInCryostat",false))
  {}

  bool FilterCryostatNus::filter(art::Event& evt){
    //get the list of particles from this event
    art::ServiceHandle<geo::Geometry> geom;

    std::vector< art::Handle< std::vector<simb::MCTruth> > > allmclists;
    evt.getManyByType(allmclists);

    bool inCryostatNu=false;
    for(size_t mcl = 0; mcl < allmclists.size(); ++mcl){
      art::Handle< std::vector<simb::MCTruth> > mclistHandle = allmclists[mcl];
      for(size_t m = 0; m < mclistHandle->size(); ++m){
        art::Ptr<simb::MCTruth> mct(mclistHandle, m);

        //get nu, does it end in cyrostat?
        for(int ipart=0;ipart<mct->NParticles();ipart++){
          if(abs(mct->GetParticle(ipart).PdgCode())==12||
             abs(mct->GetParticle(ipart).PdgCode())==14||
             abs(mct->GetParticle(ipart).PdgCode())==16){
             const TLorentzVector& end4 = mct->GetParticle(ipart).EndPosition();
             double endpointa[]={end4[0],end4[1],end4[2]};
             unsigned int cstat;
             try{
              geom->PositionToCryostat(endpointa,cstat);
              inCryostatNu=true;
             }catch(...){
             }
           }
        }

       }//end loop over mctruth col

    }//end loop over all mctruth lists

    return fKeepNusInCryostat ^ (!inCryostatNu);

  }

} // namespace simfilter

namespace simfilter {

  DEFINE_ART_MODULE(FilterCryostatNus)

} // namespace simfilter

#endif // FILTER_FILTERNODIRTNUS_H