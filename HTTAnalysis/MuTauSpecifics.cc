#include "MuTauSpecifics.h"
#include "HTTAnalyzer.h"
#include "EventProxyHTT.h"


MuTauSpecifics::MuTauSpecifics(HTTAnalyzer * aAnalyzer):ChannelSpecifics(aAnalyzer){

decayModeName = "MuTau";

}
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void MuTauSpecifics::setAnalysisObjects(const EventProxyHTT & myEventProxy) {

        myAnalyzer->aLeg1 = myAnalyzer->aPair.getMuon();
        myAnalyzer->aLeg2 = myAnalyzer->aPair.getTau();

        myAnalyzer->aGenLeg1 = HTTParticle();
        myAnalyzer->aGenLeg2 = HTTParticle();

        if(myEventProxy.genLeptons &&
           myEventProxy.genLeptons->size()) {
                HTTParticle aGenTau =  myEventProxy.genLeptons->at(0);
                if(aGenTau.getProperty(PropertyEnum::decayMode)==HTTAnalyzer::tauDecayMuon) myAnalyzer->aGenLeg1 = aGenTau;
                else if(aGenTau.getProperty(PropertyEnum::decayMode)!=HTTAnalyzer::tauDecaysElectron) myAnalyzer->aGenLeg2 = aGenTau;
                if(myEventProxy.genLeptons->size()>1) {
                        aGenTau =  myEventProxy.genLeptons->at(1);
                        if(aGenTau.getProperty(PropertyEnum::decayMode)==HTTAnalyzer::tauDecayMuon) myAnalyzer->aGenLeg1 = aGenTau;
                        else if(aGenTau.getProperty(PropertyEnum::decayMode)!=HTTAnalyzer::tauDecaysElectron) myAnalyzer->aGenLeg2 = aGenTau;
                }
        }
};
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
std::pair<bool, bool> MuTauSpecifics::checkTauDecayMode(const EventProxyHTT & myEventProxy){

        bool goodGenDecayMode = false;
        bool goodRecoDecayMode = false;

        std::vector<std::string> decayNamesGen = myAnalyzer->getTauDecayName(myAnalyzer->aGenLeg2.getProperty(PropertyEnum::decayMode),
                                                                  myAnalyzer->aGenLeg1.getProperty(PropertyEnum::decayMode));
        std::vector<std::string> decayNamesReco = myAnalyzer->getTauDecayName(myAnalyzer->aLeg2.getProperty(PropertyEnum::decayMode),HTTAnalyzer::tauDecayMuon);

        for(auto it: decayNamesGen) if(it.find("Lepton1Prong")!=std::string::npos) goodGenDecayMode = true;
        for(auto it: decayNamesReco) if(it.find("Lepton1Prong")!=std::string::npos) goodRecoDecayMode = true;

        return std::pair<bool, bool>(goodGenDecayMode, goodRecoDecayMode);
}
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
void MuTauSpecifics::testAllCategories(const sysEffects::sysEffectsEnum & aSystEffect){

        for(auto && it: myAnalyzer->categoryDecisions) it = false;

        myAnalyzer->nJets30 = 0;
        for(auto itJet: myAnalyzer->aSeparatedJets) {
                if(itJet.getP4(aSystEffect).Pt()>30) ++myAnalyzer->nJets30;
        }

        myAnalyzer->nJetsInGap30 = 0;
        if(myAnalyzer->nJets30>=2) {
                for(unsigned int iJet=2; iJet<myAnalyzer->aSeparatedJets.size(); ++iJet) {
                        if( (myAnalyzer->aSeparatedJets.at(iJet).getP4().Eta()>myAnalyzer->aJet1.getP4().Eta() &&
                          myAnalyzer->aSeparatedJets.at(iJet).getP4().Eta()<myAnalyzer->aJet2.getP4().Eta()) ||
                            (myAnalyzer->aSeparatedJets.at(iJet).getP4().Eta()<myAnalyzer->aJet1.getP4().Eta() &&
                              myAnalyzer->aSeparatedJets.at(iJet).getP4().Eta()>myAnalyzer->aJet2.getP4().Eta()) ) {
                                if(myAnalyzer->aSeparatedJets.at(iJet).getP4(aSystEffect).Pt()>30) myAnalyzer->nJetsInGap30++;
                        }
                }
        }

        float jetsMass = (myAnalyzer->aJet1.getP4(aSystEffect)+myAnalyzer->aJet2.getP4(aSystEffect)).M();
        float higgsPt =  (myAnalyzer->aLeg1.getP4(aSystEffect) + myAnalyzer->aLeg2.getP4(aSystEffect) + myAnalyzer->aMET.getP4(aSystEffect)).Pt();
        bool mtSelection = myAnalyzer->aPair.getMTMuon(aSystEffect)<50;

        bool jet0_low =  myAnalyzer->aLeg2.getP4(aSystEffect).Pt()>20 && myAnalyzer->aLeg2.getP4(aSystEffect).Pt()<50 && myAnalyzer->nJets30==0;
        bool jet0_high = myAnalyzer->aLeg2.getP4(aSystEffect).Pt()>50 && myAnalyzer->nJets30==0;

        bool jet1_low = (myAnalyzer->nJets30==1 || (myAnalyzer->nJets30==2 && jetsMass<500)) &&
                        (myAnalyzer->aLeg2.getP4(aSystEffect).Pt()>30 && myAnalyzer->aLeg2.getP4(aSystEffect).Pt()<40 ||
                         myAnalyzer->aLeg2.getP4(aSystEffect).Pt()>40 && higgsPt<140);

        bool jet1_high = (myAnalyzer->nJets30==1 || (myAnalyzer->nJets30==2 && jetsMass<500)) &&
                         (myAnalyzer->aLeg2.getP4(aSystEffect).Pt()>40 && higgsPt>140);

        bool vbf_low = myAnalyzer->aLeg2.getP4(aSystEffect).Pt()>20 &&
                       myAnalyzer->nJets30==2 && jetsMass>500 &&
                       (jetsMass<800 || higgsPt<100);

        bool vbf_high = myAnalyzer->aLeg2.getP4(aSystEffect).Pt()>20 &&
                        myAnalyzer->nJets30==2 && jetsMass>800 && higgsPt>100;

        bool cpMuonSelection = myAnalyzer->aLeg1.getPCARefitPV().Perp()>myAnalyzer->nPCAMin_;
        bool cpTauSelection =  myAnalyzer->aLeg2.getPCARefitPV().Mag()>myAnalyzer->nPCAMin_;
        bool cpPi = cpMuonSelection && cpTauSelection && myAnalyzer->aLeg2.getProperty(PropertyEnum::decayMode)==HTTAnalyzer::tauDecay1ChargedPion0PiZero;
        bool cpRho = cpMuonSelection && myAnalyzer->aLeg2.getProperty(PropertyEnum::decayMode)!=HTTAnalyzer::tauDecay1ChargedPion0PiZero &&
                     myAnalyzer->isOneProng(myAnalyzer->aLeg2.getProperty(PropertyEnum::decayMode));

        //2D categories
        bool jet0 = myAnalyzer->aLeg2.getP4(aSystEffect).Perp()>30 && myAnalyzer->nJets30 == 0;
        bool boosted = myAnalyzer->aLeg2.getP4(aSystEffect).Perp()>30 && (myAnalyzer->nJets30==1 || (myAnalyzer->nJets30==2 && jetsMass < 300) || myAnalyzer->nJets30 > 2);
        bool vbf = myAnalyzer->aLeg2.getP4(aSystEffect).Perp()>30 && myAnalyzer->nJets30==2 && jetsMass>300;

        bool wSelection = myAnalyzer->aPair.getMTMuon(aSystEffect)>80 && myAnalyzer->aLeg1.getProperty(PropertyEnum::combreliso)<0.15;
        bool ttSelection =  myAnalyzer->aPair.getMTMuon(aSystEffect)>150;
        bool muonAntiIso = myAnalyzer->aLeg1.getProperty(PropertyEnum::combreliso)>0.15 && myAnalyzer->aLeg1.getProperty(PropertyEnum::combreliso)<0.30;
        bool muonIso = myAnalyzer->aLeg1.getProperty(PropertyEnum::combreliso)<0.15;

        myAnalyzer->categoryDecisions[(int)HTTAnalyzer::jet0_low] = muonIso && mtSelection && jet0_low;
        myAnalyzer->categoryDecisions[(int)HTTAnalyzer::jet0_high] = muonIso && mtSelection && jet0_high;

        myAnalyzer->categoryDecisions[(int)HTTAnalyzer::jet1_low] = muonIso && mtSelection && jet1_low;
        myAnalyzer->categoryDecisions[(int)HTTAnalyzer::jet1_high] = muonIso && mtSelection && jet1_high;

        myAnalyzer->categoryDecisions[(int)HTTAnalyzer::vbf_low] = muonIso && mtSelection && vbf_low;
        myAnalyzer->categoryDecisions[(int)HTTAnalyzer::vbf_high] = muonIso && mtSelection && vbf_high;

        myAnalyzer->categoryDecisions[(int)HTTAnalyzer::jet0] = muonIso && mtSelection && jet0;
        myAnalyzer->categoryDecisions[(int)HTTAnalyzer::CP_Pi] = muonIso && mtSelection && cpPi;
        myAnalyzer->categoryDecisions[(int)HTTAnalyzer::CP_Rho] = muonIso && mtSelection && cpRho;
        myAnalyzer->categoryDecisions[(int)HTTAnalyzer::boosted] = muonIso && mtSelection && boosted;
        myAnalyzer->categoryDecisions[(int)HTTAnalyzer::vbf] = muonIso && mtSelection && vbf;

        myAnalyzer->categoryDecisions[(int)HTTAnalyzer::wjets_jet0] = muonIso && wSelection && jet0;
        myAnalyzer->categoryDecisions[(int)HTTAnalyzer::wjets_boosted] = muonIso && wSelection && boosted;
        myAnalyzer->categoryDecisions[(int)HTTAnalyzer::wjets_vbf] = muonIso && wSelection && vbf;

        myAnalyzer->categoryDecisions[(int)HTTAnalyzer::antiiso_jet0] = muonAntiIso && mtSelection && jet0;
        myAnalyzer->categoryDecisions[(int)HTTAnalyzer::antiiso_boosted] = muonAntiIso && mtSelection && boosted;
        myAnalyzer->categoryDecisions[(int)HTTAnalyzer::antiiso_vbf] = muonAntiIso && mtSelection && vbf;

        myAnalyzer->categoryDecisions[(int)HTTAnalyzer::W] = muonIso && wSelection;
        myAnalyzer->categoryDecisions[(int)HTTAnalyzer::TT] = muonIso && ttSelection;

}
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
