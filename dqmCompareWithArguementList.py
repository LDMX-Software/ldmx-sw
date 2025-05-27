import ROOT, sys, os, time, re, numpy
from optparse import OptionParser
parser = OptionParser(usage="Usage: python3 %prog sample.txt")
(opt,args) = parser.parse_args()

sampleInFile = sys.argv[1]


ROOT.gROOT.SetBatch(True)
#ROOT.gStyle.SetPalette(1)
ROOT.gStyle.SetPadRightMargin(.15)
ROOT.gStyle.SetPadTopMargin(0.1)
ROOT.gStyle.SetPadBottomMargin(0.14)
ROOT.gStyle.SetPadLeftMargin(0.15)

SamplesArray = []

with open(sampleInFile, "r") as a_file:
  for line in a_file:
    stripped_line = line.strip()
    SamplesArray.append(stripped_line)

fileInArray = []
for sample in SamplesArray:
  fileInArray.append(ROOT.TFile.Open(sample))

f = fileInArray[0]
# print("FILE KEYS:")
# print(fileInArray[0].GetListOfKeys())
for i in range(0, fileInArray[0].GetListOfKeys().GetEntries()):
  dirname = f.GetListOfKeys().At(i).GetName()
  curr_dir = f.GetDirectory(dirname)
  # print("dirname: "+dirname)
  if not (curr_dir) :
    continue
  for i in range(0, curr_dir.GetListOfKeys().GetEntries()):
      # Match the plot of interest
      keyname = curr_dir.GetListOfKeys().At(i).GetName()
      # if not ("CutFlow" in keyname):  continue
      if  not ("EcalMipTrackingFeatures" in keyname or "EcalShowerFeatures" in keyname):  continue
      if  ("SeedTagger" in keyname or "SeedRecoil" in keyname):  continue
      keyname2 = keyname
      curr_dir2 = f.GetDirectory(dirname+"/"+keyname)
      if True :
          keyname = curr_dir.GetListOfKeys().At(i).GetName()
          # print(dirname+"/"+keyname)
          curr_dir2 = fileInArray[0].GetDirectory(dirname+"/"+keyname)
          newname = dirname+"/"+keyname
          obj = fileInArray[0].Get(newname)
          obj.SetMarkerStyle(20)
          
          tex2 = ROOT.TLatex(0.15,0.92,"LDMX")
          tex2.SetNDC()
          tex2.SetTextFont(61)
          tex2.SetTextSize(0.0675)
          tex2.SetLineWidth(2)

          
          tex3 = ROOT.TLatex(0.33,0.92,"Simulation") # for square plots
          tex3.SetNDC()
          tex3.SetTextFont(52)
          tex3.SetTextSize(0.0485)
          tex3.SetLineWidth(2)
          
          tex4 = ROOT.TLatex(0.64,0.92,"10000 EOT (8 GeV)")
          tex4.SetNDC()
          tex4.SetTextFont(52)
          tex4.SetTextSize(0.03)
          tex4.SetLineWidth(2)
          
          tex5 = ROOT.TLatex()
#          tex5 = ROOT.TLatex(0.07,0.04,"Code version: "+codeVersion)

          overFlowText = ROOT.TLatex(0.836,0.15,"OF")
#          overFlowText.SetTextAngle(-30)
          overFlowText.SetNDC()
          overFlowText.SetTextFont(52)
          overFlowText.SetTextSize(0.01)
          overFlowText.SetLineWidth(2)
          
          overFlowText = ROOT.TLatex(0.836,0.15,"OF")
#          overFlowText.SetTextAngle(-30)
          overFlowText.SetNDC()
          overFlowText.SetTextFont(52)
          overFlowText.SetTextSize(0.01)
          overFlowText.SetLineWidth(2)
          
          if any(substring in keyname2 for substring in ["_regionA_", "_regionB_", "_regionC_", "_regionD_", "VR1_Mass",  "VR2_Mass", "VR3_Mass", "SR1_Mass",  "SR2_Mass", "SR3_Mass"]) : continue
          if obj.InheritsFrom("TObject"):
              if not os.path.exists(os.path.dirname("Compare"+sampleInFile[:-5]+"/a.png")):
                print("Create dir")
                os.makedirs(os.path.dirname("Compare"+sampleInFile[:-5]+"/"))
#              print(keyname2)


              if (obj.GetEntries() == 0 ) : continue

              OFbin = 0
              if (any(substring in keyname2 for substring in ["Ias", "ProbQ", "Beta", "CutFlow", "Qual", "Eta", "Type"])):
                OFbin = obj.GetNbinsX()
              else :
                OFbin = obj.GetNbinsX()+1
                
#              if (any(substring in keyname2 for substring in ["BetaGamma"])):
#                OFbin = obj.GetNbinsX()+1

              if (obj.ClassName() == "TH3F" or obj.ClassName() == "TH3D"):
                continue
                obj.SetMarkerStyle(20)
                obj.GetXaxis().SetRange(bin,bin)
                obj.Project3D("ZY").Draw("COLZ")
              if (obj.ClassName() == "TH2F" and  (("Trigger" in keyname2) and ( "Vs" in keyname2) or "VsBeta" in keyname2)) :
                continue
                canvasString = 'canvas1'+str(i)
                canvas = ROOT.TCanvas(canvasString, canvasString, 800,800)
                legendStartX = 0.0
                legendEntryForZero = SamplesArray[0][:-5]
                if ((len(legendEntryForZero)) < 25) :
                  legendStartX = 0.6
                if ((len(legendEntryForZero)) >= 25) :
                  legendStartX = 0.5
                if ((len(legendEntryForZero)) >= 40) :
                  legendStartX = 0.3
                if ((len(legendEntryForZero)) >= 55) :
                  legendStartX = 0.2
                legend = ROOT.TLegend(legendStartX,.65,.75,.89,"","brNDC")
                legend.SetTextFont(42)
                legend.SetTextSize(0.017)
                legend.SetBorderSize(1)
                legend.SetBorderSize(0)
                legend.SetLineColor(1)
                legend.SetLineStyle(1)
                legend.SetLineWidth(1)
                legend.SetFillColor(0)
                legend.SetFillStyle(1001)

                
                histoArrayProfY = []
                k = 0
                for fileIn in fileInArray:
                  k = k + 1
                  projXName = keyname2 + str(k)
                  if (fileIn.Get(newname)) : histoArrayProfY.append(fileIn.Get(newname).ProfileY().ProjectionX(projXName))
                indexNew = -1
                for index in range(0, len(histoArrayProfY)):
                  histoArrayProfY[index].SetStats(0)
                  histoArrayProfY[index].SetMarkerStyle(20)
                  legendEntry = SamplesArray[index][:-5]
                  legend.AddEntry(histoArrayProfY[index],legendEntry,"LP")
                  if (index>-1):
                    indexNew = index+2
                  if (indexNew==10) :
                    indexNew = 40
                  elif (indexNew==11) :
                    indexNew = 46
                  elif (indexNew==12) :
                    indexNew = 41
                  elif (indexNew==13) :
                    indexNew = 30
                  elif (indexNew==14) :
                    indexNew = 42
                  histoArrayProfY[index].SetLineColor(indexNew)
                  histoArrayProfY[index].SetMarkerColor(indexNew)
#                  histoArray[index].SetFillColor(indexNew)
                  histoArrayProfY[index].SetTitle("")
                  histoArrayProfY[index].GetYaxis().SetTitle("Efficiency")
                  histoArrayProfY[index].SetMaximum(1.6)
                  histoArrayProfY[index].DrawClone("SAME")
                legend.Draw("SAME")
                tex2.Draw("SAME")
                tex3.Draw("SAME")
                tex4.Draw("SAME")
                tex5.Draw("SAME")
                canvas.SaveAs("Compare"+sampleInFile[:-5]+"/"+keyname2+"_profileY.png")

              # Now do the 1D histos
              if (obj.ClassName() == "TH1F" or obj.ClassName() == "TProfile"): # and "BS_" in keyname2):
                canvasString = 'canvas'+str(i)
                canvas = ROOT.TCanvas(canvasString, canvasString, 800,800)
                                #TADA
                legendStartX = 0.0
                legendEntryForZero = SamplesArray[0][:-5]

#                print((len(legendEntryForZero)))
                if ((len(legendEntryForZero)) < 25) :
                  legendStartX = 0.6
                if ((len(legendEntryForZero)) >= 25) :
                  legendStartX = 0.5
                if ((len(legendEntryForZero)) >= 40) :
                  legendStartX = 0.3
                if ((len(legendEntryForZero)) >= 55) :
                  legendStartX = 0.2
                legend =  ROOT.TLegend(legendStartX,.65,.85,.89,"","brNDC")
                legend.SetTextFont(42)
                legend.SetTextSize(0.017)
                legend.SetBorderSize(1)
                legend.SetBorderSize(0)
                legend.SetLineColor(1)
                legend.SetLineStyle(1)
                legend.SetLineWidth(1)
                legend.SetFillColor(0)
#                legend.SetFillStyle(1001)
                legend.SetFillStyle(0)
                
                histoArray = []
                for fileIn in fileInArray:
                  if (fileIn.Get(newname)) : histoArray.append(fileIn.Get(newname))
                for index in range(0, len(histoArray)):
                  histoArray[index].SetStats(0)
                  histoArray[index].SetMarkerStyle(20)
                  histoArray[index].GetXaxis().SetRange(1,OFbin)
                  legendEntry = SamplesArray[index][:-5]
                  legend.AddEntry(histoArray[index],legendEntry,"LP")
                  indexNew = -1
                  if (index>-1):
                    indexNew = index+2
                  if (indexNew==10) :
                    indexNew = 40
                  elif (indexNew==11) :
                    indexNew = 46
                  elif (indexNew==12) :
                    indexNew = 41
                  elif (indexNew==13) :
                    indexNew = 30
                  elif (indexNew==14) :
                    indexNew = 42
                  histoArray[index].SetLineColor(indexNew)
                  histoArray[index].SetMarkerColor(indexNew)
#                  histoArray[index].SetFillColor(indexNew)
                  histoArray[index].SetTitle("")
                  max = 0.0
#                  print(keyname2)
                  for index2 in range(0, len(histoArray)):
                    if not (histoArray[index2]) : continue
                    max = numpy.maximum(max,histoArray[index2].GetMaximum())
#                  histoArray[index].GetYaxis().SetTitle("Tracks/bin")
#                  histoArray[index].GetXaxis().SetTitle(keyname2)
                  binWidth = histoArray[index].GetXaxis().GetBinWidth(1)
                  firstBinLocXCent = histoArray[index].GetXaxis().GetBinCenter(1)
                  overFlowLocXCent = histoArray[index].GetXaxis().GetBinCenter(histoArray[index].GetNbinsX()+ 1)
                  overFlowLocX = (overFlowLocXCent - (binWidth)/2)
                  firstBinLoxX = abs(firstBinLocXCent - (binWidth)/2)
                  
                  overFlowLine = ROOT.TLine()
#                  overFlowLine.SetNDC()
                  overFlowLine.SetLineWidth(2)
                  overFlowLine.SetLineStyle(ROOT.kDashed)
                  histoArray[index].SetMaximum(max*1.5)
                  histoArray[index].SetMinimum(0)
                  histoArray[index].Draw("SAME")

                legend.Draw("SAME")
                tex2.Draw("SAME")
                tex3.Draw("SAME")
                tex4.Draw("SAME")
                tex5.Draw("SAME")
                overFlowText.Draw("SAME")
                overFlowLine.DrawLine(overFlowLocX,histoArray[0].GetMinimum(),overFlowLocX,histoArray[0].GetMaximum())
                canvas.SaveAs("Compare"+sampleInFile[:-5]+"/"+keyname2+".png")

                # repeat everything with LOG scale
                cstackPlotsLogString = 'cstackPlotsLog'+str(i)
                cstackPlots = ROOT.TCanvas(cstackPlotsLogString, cstackPlotsLogString, 800,800)
                cstackPlots.SetLogy()
                for index in range(0, len(histoArray)):
                  histoArray[index].GetXaxis().SetRange(1,OFbin)
                  if ("HltMatchTrackLevel" in keyname2 or "TriggerGenMatch" in keyname2) :
                     histoArray[index].Draw("SAMEHISTOTEXT00")
                  else :
                    histoArray[index].Draw("SAME")
#                  histoArray[index].Draw("SAME")
                  if ("CutFlow" in keyname2 or "pfType"  in keyname2 ) :
                     histoArray[index].SetMaximum(100)
                     histoArray[index].SetMinimum(0.0001)
                  if (keyname2== "EventCutFlow") :
                     histoArray[index].SetStats(0)
                     if ("SingleMuon" in SamplesArray[index]) :
                       histoArray[index].SetMinimum(300000)
                       histoArray[index].SetMaximum(max*10000)
                       print(str(SamplesArray[index][14:SamplesArray[index].find("Code")-1])+": "+str(histoArray[index].GetBinContent(17)))
                     else :
                      histoArray[index].SetMaximum(max*10000)
                      histoArray[index].SetMinimum(0.01)

                     
                     ROOT.gStyle.SetPaintTextFormat(".2g")
                     histoArray[index].Draw("SAMEHISTOTEXT25")

                  elif ("ProbQ" in keyname2) :
                    histoArray[index].SetMaximum(max*1000000)
                    histoArray[index].SetMinimum(0.000001)
                  else :
                    histoArray[index].SetMaximum(max*10000)
                    histoArray[index].SetMinimum(0.000001)
                legend.Draw("SAME")
                tex2.Draw("SAME")
                tex3.Draw("SAME")
                tex4.Draw("SAME")
                tex5.Draw("SAME")
                
                overFlowText.Draw("SAME")
                overFlowLine.DrawLine(overFlowLocX,histoArray[0].GetMinimum(),overFlowLocX,max*10000)
                cstackPlots.SaveAs("Compare"+sampleInFile[:-5]+"/"+keyname2+"_log.png")
                
                cstackPlotsNormString = 'cstackPlotsNorm'+str(i)
                cstackPlotsNorm = ROOT.TCanvas(cstackPlotsNormString, cstackPlotsLogString, 800,800)
                maxNorm = 0.
                for index in range(0, len(histoArray)):
                  yAxisTitle = histoArray[index].GetYaxis().GetTitle()
                  if (any(substring in keyname2 for substring in ["Ias", "ProbQ", "Beta", "CutFlow", "Qual", "Eta", "Type"])):
                    int = histoArray[index].Integral(1,histoArray[index].GetNbinsX())
                  else :
                    int = histoArray[index].Integral(1,histoArray[index].GetNbinsX()+1)
                    
                  if (any(substring in keyname2 for substring in ["CandidateType", "HltMatchTrackLevel", "TriggerGenMatch", "MuonType"])):
                    int = histoArray[index].GetBinContent(1)
                  if (int>0) :
                    histoArray[index].Scale(1/int)
                  histoArray[index].GetYaxis().SetTitle("Normalized " + yAxisTitle)
                  maxNorm = numpy.maximum(maxNorm,histoArray[index].GetMaximum())
                  
                  if ("CandidateType" in keyname2 or "HltMatchTrackLevel" in keyname2 or "TriggerGenMatch" in keyname2) :
                    histoArray[index].Draw("SAMEHISTOTEXT00")
                  if ("EventCutFlow" in keyname2) :
                    histoArray[index].Draw("SAMEHISTOTEXT25")
                    legendEntry2 = SamplesArray[index][:-5]

                    print(" |  | "+legendEntry2)
                    print(" | --- | --- | ")
                    for binIndex in range(1,histoArray[index].GetNbinsX()-3):
                      eff = histoArray[index].GetBinContent(binIndex)/histoArray[index].GetBinContent(1)
                      print(" | " + str(histoArray[index].GetXaxis().GetBinLabel(binIndex)) + " | " + str(round(eff,3))+ " | " )
                    #TODO
                  if ("GenBeta" in keyname2) :
                    print("i =" +str(index) + "# #beta[0.5-0.75] " + str(round(histoArray[index].Integral(histoArray[index].GetXaxis().FindBin(0.5),histoArray[index].GetXaxis().FindBin(0.75)),2)))
                    histoArray[index].Draw("SAME")
#                  if ("HSCPCandidateType" in keyname2) :
#                    print(str(round(100*histoArray[index].GetBinContent(4),1))+"%")
                  else :
                    histoArray[index].Draw("SAME")
                histoArray[0].SetMaximum(maxNorm*1.6)
                histoArray[0].SetMinimum(0) # was -maxNorm*1.6/20
                if ("HltMatchTrackLevel" in keyname2) :
                    histoArray[0].GetYaxis().SetRangeUser(0.60,1.25)
                    
                legend.Draw("SAME")
                tex2.Draw("SAME")
                tex3.Draw("SAME")
                tex4.Draw("SAME")
                tex5.Draw("SAME")
                
                overFlowText.Draw("SAME")
                overFlowLine.DrawLine(overFlowLocX,histoArray[0].GetMinimum(),overFlowLocX,histoArray[0].GetMaximum())
                cstackPlotsNorm.SaveAs("Compare"+sampleInFile[:-5]+"/"+keyname2+"_Norm.png")
                
                # -----------------------------------------------------------------------------------------
                
                cstackPlotsNormLogString = 'cstackPlotsNormLog'+str(i)
                cstackPlotsNormLog = ROOT.TCanvas(cstackPlotsNormLogString, cstackPlotsNormLogString, 800,800)
                cstackPlotsNormLog.SetLogy()
                maxNorm = 0.
                for index in range(0, len(histoArray)):
                  yAxisTitle = histoArray[index].GetYaxis().GetTitle()
                  int = histoArray[index].Integral(1,histoArray[index].GetNbinsX()+1)
                  if ("CandidateType" in keyname2 or "HltMatchTrackLevel" in keyname2 or "TriggerGenMatch" in keyname2) :
                    int = histoArray[index].GetBinContent(1)
                  if (int>0) :
                    histoArray[index].Scale(1/int)
                  maxNorm = numpy.maximum(maxNorm,histoArray[index].GetMaximum())
                  histoArray[index].GetXaxis().SetRange(1,OFbin)
                  if ("CandidateType" in keyname2 or "HltMatchTrackLevel" in keyname2 or "TriggerGenMatch" in keyname2) :
                    histoArray[index].SetMaximum(1.6)
                    histoArray[index].Draw("SAMEHISTOTEXT00")
                  if ("RecoHSCParticleType" in keyname2) :
                    histoArray[index].Draw("SAMEHISTOTEXT00")
#                    print(histoArray[index].GetBinContent(1))
                  else :
                    histoArray[index].Draw("SAME")
                histoArray[0].SetMaximum(maxNorm*10000)
                legend.Draw("SAME")
                tex2.Draw("SAME")
                tex3.Draw("SAME")
                tex4.Draw("SAME")
                tex5.Draw("SAME")

                overFlowText.Draw("SAME")
                overFlowLine.DrawLine(overFlowLocX,histoArray[0].GetMinimum(),overFlowLocX,maxNorm*10000)
                cstackPlotsNormLog.SaveAs("Compare"+sampleInFile[:-5]+"/"+keyname2+"_NormLog.png")
