/*
** Copyright (C) 2000 George Tzanetakis <gtzan@cs.princeton.edu>
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/** 
    bextract: batch feature extraction 
*/

#include <cstdio>
#include "Collection.h"
#include "MarSystemManager.h"
#include "Accumulator.h"
#include "Fanout.h"
#include "CommandLineOptions.h"
#include "TimeLine.h"
#include "FileName.h"

#include <string> 
using namespace std;
using namespace Marsyas;

int helpopt;
int usageopt;
int normopt;

bool tline;

mrs_natural offset = 0;
mrs_natural duration = 1000 * 44100;
mrs_natural memSize = 1;
mrs_natural winSize = 512;
mrs_natural hopSize = 512;
mrs_real samplingRate_ = 22050.0;
mrs_natural accSize_ = 1298;
mrs_real start = 0.0;
mrs_real length = 30.0;
mrs_real gain = 1.0;
mrs_bool pluginMute = 0.0;

#define DEFAULT_EXTRACTOR "STFT" 
#define DEFAULT_CLASSIFIER  "GS"

string pluginName = EMPTYSTRING;
string wekafname = EMPTYSTRING;
string filefeaturename = EMPTYSTRING;
string extractorName = EMPTYSTRING;
string classifierName = EMPTYSTRING;

CommandLineOptions cmd_options;

// map table with pointers to the functions that create
// each specific and supported feature extraction MarSystem
typedef MarSystem* (*FeatureExtractorFactory)();
map<string,FeatureExtractorFactory >  featExtractors;
map<string, string> featExtractorDesc;

//////////////////////////////////////////////////////////////////////////
// Feature Extractors creation Functions
//////////////////////////////////////////////////////////////////////////
MarSystem* createExtractorFromFile()
{
  MarSystemManager mng;

  //this opens a .mpl file and creates in run-time 
  //the MarSystem network for feature extraction.
  //NOTE:
  //the network should not contain any source MarSystem
  //since that is added by bextract
  ifstream mplFile(extractorName.c_str());
	
  return mng.getMarSystem(mplFile);
}

MarSystem* createBEATextrator()
{
  MarSystemManager mng;

  MarSystem* extractor = mng.create("Series", "beatExtrator");  
  extractor->addMarSystem(mng.create("SoundFileSource", "src1"));
  extractor->addMarSystem(mng.create("ShiftInput", "si"));
  extractor->addMarSystem(mng.create("DownSampler", "initds"));
  extractor->addMarSystem(mng.create("WaveletPyramid", "wvpt"));
  extractor->addMarSystem(mng.create("WaveletBands", "wvbnds"));
  extractor->addMarSystem(mng.create("FullWaveRectifier", "fwr"));
  extractor->addMarSystem(mng.create("OnePole", "lpf"));
  extractor->addMarSystem(mng.create("Norm", "norm"));
  extractor->addMarSystem(mng.create("Sum", "sum"));
  extractor->addMarSystem(mng.create("DownSampler", "ds"));
  extractor->addMarSystem(mng.create("AutoCorrelation", "acr"));
  extractor->addMarSystem(mng.create("Peaker", "pkr"));
  extractor->addMarSystem(mng.create("MaxArgMax", "mxr"));
  extractor->addMarSystem(mng.create("PeakPeriods2BPM", "p2bpm"));
  extractor->addMarSystem(mng.create("Histogram", "histo"));
  // extractor->addMarSystem(mng.create("PlotSink", "psink"));
  // extractor->addMarSystem(mng.create("Reassign", "reassign"));
  extractor->addMarSystem(mng.create("BeatHistoFeatures", "bhf"));
	
  return extractor;
}

MarSystem* createSTFTextractor()
{
  MarSystemManager mng;

  MarSystem* extractor = mng.create("Fanout", "STFTextractor");

  // Time domain features
  extractor->addMarSystem(mng.create("ZeroCrossings", "zcrs"));

  // Frequency domain features
  MarSystem* spectralShape = mng.create("Series", "spectralShape");
  spectralShape->addMarSystem(mng.create("PowerSpectrumNet","powerSpect"));
  // Spectrum Shape descriptors
  MarSystem* spectrumFeatures = mng.create("Fanout", "spectrumFeatures");
  spectrumFeatures->addMarSystem(mng.create("Centroid", "cntrd"));
  spectrumFeatures->addMarSystem(mng.create("Rolloff", "rlf"));      
  spectrumFeatures->addMarSystem(mng.create("Flux", "flux"));

  spectralShape->addMarSystem(spectrumFeatures);

  extractor->addMarSystem(spectralShape);

  extractor->linkctrl("mrs_natural/WindowSize", "PowerSpectrumNet/powerSpect/mrs_natural/WindowSize");

  return extractor;
}

MarSystem* createMFCCextractor()
{
  MarSystemManager mng;

  MarSystem* extractor = mng.create("Series", "MFCCextractor");
  extractor->addMarSystem(mng.create("PowerSpectrumNet","powerSpect"));
  extractor->addMarSystem(mng.create("MFCC", "mfcc"));

  extractor->linkctrl("mrs_natural/WindowSize", "PowerSpectrumNet/powerSpect/mrs_natural/WindowSize");

  return extractor;
}

MarSystem* createSTFTMFCCextractor()
{
  MarSystemManager mng;

  MarSystem* extractor = mng.create("Series", "STFTMFCCextractor");

  extractor->addMarSystem(mng.create("PowerSpectrumNet","powerSpect"));
  // Spectrum Shape descriptors
  MarSystem* spectrumFeatures = mng.create("Fanout", "spectrumFeatures");
  spectrumFeatures->addMarSystem(mng.create("Centroid", "cntrd"));
  spectrumFeatures->addMarSystem(mng.create("Rolloff", "rlf"));      
  spectrumFeatures->addMarSystem(mng.create("Flux", "flux"));
  spectrumFeatures->addMarSystem(mng.create("MFCC", "mfcc"));
  extractor->addMarSystem(spectrumFeatures);

  extractor->linkctrl("mrs_natural/WindowSize", "PowerSpectrumNet/powerSpect/mrs_natural/WindowSize");

  return extractor;
}

MarSystem* createSCFextractor()
{
  MarSystemManager mng;

  MarSystem* extractor = mng.create("Series", "SCFextractor");
  extractor->addMarSystem(mng.create("PowerSpectrumNet","powerSpect"));
  extractor->addMarSystem(mng.create("SCF", "scf"));

  extractor->linkctrl("mrs_natural/WindowSize", "PowerSpectrumNet/powerSpect/mrs_natural/WindowSize");

  return extractor;
}

MarSystem* createSFMextractor()
{
  MarSystemManager mng;

  MarSystem* extractor = mng.create("Series", "SFMextractor");
  extractor->addMarSystem(mng.create("PowerSpectrumNet","powerSpect"));
  extractor->addMarSystem(mng.create("SFM", "sfm"));

  extractor->linkctrl("mrs_natural/WindowSize", "PowerSpectrumNet/powerSpect/mrs_natural/WindowSize");

  return extractor;
}

MarSystem* createSFMSCFextractor()
{
  MarSystemManager mng;

  MarSystem* extractor = mng.create("Series", "SFMSCFextractor");

  extractor->addMarSystem(mng.create("PowerSpectrumNet","powerSpect"));
  // Spectrum Shape descriptors
  MarSystem* spectrumFeatures = mng.create("Fanout", "spectrumFeatures");
  spectrumFeatures->addMarSystem(mng.create("SFM", "sfm"));
  spectrumFeatures->addMarSystem(mng.create("SCF", "scf"));

  extractor->addMarSystem(spectrumFeatures);

  extractor->linkctrl("mrs_natural/WindowSize", "PowerSpectrumNet/powerSpect/mrs_natural/WindowSize");

  return extractor;
}

MarSystem* createLSPextractor()
{
  MarSystemManager mng;

  mrs_natural order = 18; //12 order LSP
  cout << "LSP order = " << order << endl;

  MarSystem* extractor = mng.create("Series","LSPextractor");

  //first add a LPC calculation net
  extractor->addMarSystem(mng.create("LPCnet", "lpcNet"));
  //add the LSP calculation
  extractor->addMarSystem(mng.create("LSP", "lsp"));

  //must create a link otherwise the outmost composite will know nothing
  //about this order change (which affects output nr of observations) and
  //consequently will not update the network accordingly!
  extractor->linkctrl("mrs_natural/order", "LPCnet/lpcNet/mrs_natural/order");
  extractor->updctrl("mrs_natural/order", order); 

  extractor->linkctrl("mrs_natural/WindowSize", "LPCnet/lpcNet/mrs_natural/WindowSize");

  return extractor;
}

MarSystem* createLPCCextractor()
{
  MarSystemManager mng;

  mrs_natural order = 12; //12 order LPCC
  cout << "LPCC order = " << order << endl;

  MarSystem* extractor = mng.create("Series","LPCCextractor");

  //first add a LPC calculation net
  extractor->addMarSystem(mng.create("LPCnet", "lpcNet"));
  //add the LPCC calculation
  extractor->addMarSystem(mng.create("LPCC", "lpcc"));

  //must create a link otherwise the outmost composite will know nothing
  //about this order change (which affects output nr of observations) and
  //consequently will not update the network accordingly!
  extractor->linkctrl("mrs_natural/order", "LPCnet/lpcNet/mrs_natural/order");
  extractor->updctrl("mrs_natural/order", order);

  extractor->linkctrl("mrs_natural/WindowSize", "LPCnet/lpcNet/mrs_natural/WindowSize");

  return extractor;
}

void 
printUsage(string progName)
{
  MRSDIAG("bextract.cpp - printUsage");
  cerr << "Usage : " << progName << " [-e extractor] [-h help] [-o offset(samples)] [-d duration(samples)] [-s start(seconds)] [-l length(seconds)] [-m memory]  [-u usage] collection1 collection2 ... collectionN" << endl;
  cerr << endl;
}

void 
printHelp(string progName)
{
  MRSDIAG("bextract.cpp - printHelp");
  cerr << "bextract, MARSYAS, Copyright George Tzanetakis " << endl;
  cerr << "--------------------------------------------" << endl;
  cerr << "Prints information about the sound files provided as arguments " << endl;
  cerr << endl;
  cerr << "Usage : " << progName << "[-c collection] file1 file2 file3" << endl;
  cerr << endl;
  cerr << "where file1, ..., fileN are sound files in a Marsyas supported format" << endl;
  cerr << "Help Options:" << endl;
  cerr << "-u --usage       : display short usage info" << endl;
  cerr << "-h --help        : display this information " << endl;
  cerr << "-e --extractor   : exactor " << endl;
  cerr << "-o --offset      : playback start offset in samples " << endl;
  cerr << "-p --plugin      : output plugin name " << endl;
  cerr << "-pm --pluginmute : mute the plugin " << endl;
  cerr << "-d --duration    : playback duration in samples     " << endl;
  cerr << "-s --start       : playback start offest in seconds " << endl;
  cerr << "-l --length      : playback length in seconds " << endl;
  cerr << "-m --memory      : memory size " << endl;
  cerr << "-sr --samplingrate : sampling rate " << endl;
  cerr << "-w --weka        : weka .arff filename " << endl;
  cerr << "-ws --nwinsamples: analysis window size in samples " << endl;
  cerr << "-hp --nhopsamples: analysis hop size in samples " << endl;
  cerr << "-t --timeline    : flag 2nd input collection as timelines for the 1st collection";
  cerr << endl;
	
  cerr << "Available extractors: " << endl;
  cerr << "---------------------" << endl;
  map<string, FeatureExtractorFactory >::const_iterator it;
  for(it = featExtractors.begin(); it != featExtractors.end(); ++it)
    {
      cerr << it->first << ": " << featExtractorDesc[it->first] << endl;
    }
  cerr << endl;
  cerr << "NOTE:" << endl;
  cerr << "All extractors calculate means and variances over a memory size window" << endl;
  cerr << "SV can be appended in front of any extractor to extract a single vector (mean, variances) over a 30-second clip (for example SVSTF) " << endl;


}

void 
tempo_histoSumBands(MarSystem* total1, string sfName, realvec& beatfeatures,
		    realvec& iwin, realvec& estimate)
{
  estimate.setval(0.0);

  mrs_real srate;

  // prepare network 

  // update the controls 
  // input filename with hopSize/winSize 

  total1->updctrl("SoundFileSource/src1/mrs_string/filename", sfName);

  srate = total1->getctrl("SoundFileSource/src1/mrs_real/israte")->toReal();

  mrs_natural ifactor = 8;
  total1->updctrl("DownSampler/initds/mrs_natural/factor", ifactor);  



  mrs_natural winSize = (mrs_natural) ((srate / 22050.0) * 2 * 65536);
  mrs_natural hopSize = winSize / 16;

  offset = (mrs_natural) (start * srate);
  
  // only do 30 seconds 
  duration = (mrs_natural) (30.0 * srate);

  total1->updctrl("mrs_natural/inSamples", hopSize);
  total1->updctrl("SoundFileSource/src1/mrs_natural/pos", offset);      
  total1->updctrl("SoundFileSource/src1/mrs_natural/inSamples", hopSize);
  total1->updctrl("ShiftInput/si/mrs_natural/WindowSize", winSize);
  total1->updctrl("ShiftInput/si/mrs_bool/reset", true);
  total1->updctrl("MaxArgMax/mxr/mrs_natural/nMaximums", 3);  
  
  // wavelt filterbank envelope extraction controls 
  total1->updctrl("WaveletPyramid/wvpt/mrs_bool/forward", true);
  total1->updctrl("OnePole/lpf/mrs_real/alpha", 0.99f);
  mrs_natural factor = 32;
  total1->updctrl("DownSampler/ds/mrs_natural/factor", factor);  

  srate = total1->getctrl("DownSampler/initds/mrs_real/osrate")->toReal();  


  // Peak picker 4BPMs at 60BPM resolution from 50 BPM to 250 BPM 
  mrs_natural pkinS = total1->getctrl("Peaker/pkr/mrs_natural/onSamples")->toNatural();
  mrs_real peakSpacing = ((mrs_natural)(srate * 60.0 / (factor *60.0)) - 
			  (mrs_natural)(srate * 60.0 / (factor *62.0))) / (pkinS * 1.0);
  mrs_natural peakStart = (mrs_natural)(srate * 60.0 / (factor * 200.0));
  mrs_natural peakEnd   = (mrs_natural)(srate * 60.0 / (factor * 50.0));
  total1->updctrl("Peaker/pkr/mrs_real/peakSpacing", peakSpacing);
  total1->updctrl("Peaker/pkr/mrs_real/peakStrength", 0.5);
  total1->updctrl("Peaker/pkr/mrs_natural/peakStart", peakStart);
  total1->updctrl("Peaker/pkr/mrs_natural/peakEnd", peakEnd);
  total1->updctrl("Peaker/pkr/mrs_real/peakGain", 2.0);
  total1->updctrl("Histogram/histo/mrs_natural/startBin", 0);
  total1->updctrl("Histogram/histo/mrs_natural/endBin", 250);
  total1->updctrl("Histogram/histo/mrs_bool/reset", true);

  // prepare vectors for processing 
  /* realvec iwin(total->getctrl("mrs_natural/inObservations")->toNatural(), 
     total->getctrl("mrs_natural/inSamples")->toNatural());
     realvec estimate(total->getctrl("mrs_natural/onObservations")->toNatural(), 
     total->getctrl("mrs_natural/onSamples")->toNatural());
  */ 

  mrs_natural onSamples;

  int numPlayed =0;
  mrs_natural wc=0;
  mrs_natural samplesPlayed = 0;
  mrs_natural repeatId = 1;

  // vector of bpm estimate used to calculate median 
  onSamples = total1->getctrl("ShiftInput/si/mrs_natural/onSamples")->toNatural();

  total1->updctrl("SoundFileSource/src1/mrs_natural/pos", 0);

  while (total1->getctrl("SoundFileSource/src1/mrs_bool/notEmpty")->toBool())
    {
      total1->process(iwin, estimate);
      numPlayed++;
      wc ++;
      samplesPlayed += onSamples;
      // no duration specified so use all of source input 
    }

  // cout << "FINAL = " << bpms[bpms.size()-1] << endl;

  beatfeatures = estimate;
}



void 
bextract_trainStereoSPS(vector<Collection> cls, string classNames, 
		     string wekafname, mrs_natural memSize)
{
  cout << "STEREO" << endl;
  cout << "classNames = "  << classNames << endl;
  cout << "wekafname = "  << wekafname << endl;
  


  MarSystemManager mng;

  MarSystem* playbacknet = mng.create("Series", "playbacknet");
  playbacknet->addMarSystem(mng.create("SoundFileSource", "src"));
  // playbacknet->addMarSystem(mng.create("AudioSink", "dest"));
  
  
  MarSystem* stereobranches = mng.create("Parallel", "stereobranches");
  MarSystem* left = mng.create("Series", "left");
  MarSystem* right = mng.create("Series", "right");
  
  left->addMarSystem(mng.create("Windowing", "hamleft"));
  left->addMarSystem(mng.create("Spectrum", "spkleft"));
  right->addMarSystem(mng.create("Windowing", "hamright"));
  right->addMarSystem(mng.create("Spectrum", "spkright"));

  stereobranches->addMarSystem(left);
  stereobranches->addMarSystem(right);

  playbacknet->addMarSystem(stereobranches);
  playbacknet->addMarSystem(mng.create("StereoSpectrum", "sspk"));
  playbacknet->addMarSystem(mng.create("StereoSpectrumFeatures", "sspkf"));
  playbacknet->addMarSystem(mng.create("TextureStats", "texturests"));

  MarSystem* acc = mng.create("Accumulator", "acc");
  acc->addMarSystem(playbacknet);
  
  MarSystem* statistics2 = mng.create("Fanout", "statistics2");
  statistics2->addMarSystem(mng.create("Mean", "mn"));
  statistics2->addMarSystem(mng.create("StandardDeviation", "std"));

  MarSystem* total = mng.create("Series", "total");
  total->addMarSystem(acc);
  total->updctrl("Accumulator/acc/mrs_natural/nTimes", 1000);
  total->addMarSystem(statistics2);

  total->addMarSystem(mng.create("Annotator", "ann"));
  total->addMarSystem(mng.create("WekaSink", "wsink"));

  
  total->updctrl("WekaSink/wsink/mrs_natural/nLabels", (mrs_natural)cls.size());
  total->updctrl("WekaSink/wsink/mrs_natural/downsample", 1); 
  total->updctrl("WekaSink/wsink/mrs_string/labelNames", classNames);
  total->updctrl("WekaSink/wsink/mrs_string/filename", wekafname); 

  total->updctrl("mrs_natural/inSamples", 1024);

  mrs_bool isEmpty;

  // cout << *total << endl;
  int cj,i;
  for (cj=0; cj < cls.size(); cj++)
    {
      Collection l = cls[cj];
      total->updctrl("Annotator/ann/mrs_natural/label", cj); 
      for (i=0; i < l.size(); i++)
	{
	  total->updctrl("Accumulator/acc/Series/playbacknet/SoundFileSource/src/mrs_string/filename", l.entry(i));	  
	  cout << "Processing" << l.entry(i) << endl;
	  total->tick();	  
	}
    }
  
  
}


void 
bextract_trainStereoSPSMFCC(vector<Collection> cls, string classNames, 
		     string wekafname, mrs_natural memSize)
{
  cout << "STEREO" << endl;
  cout << "classNames = "  << classNames << endl;
  cout << "wekafname = "  << wekafname << endl;
  

  MarSystemManager mng;

  MarSystem* playbacknet = mng.create("Series", "playbacknet");
  playbacknet->addMarSystem(mng.create("SoundFileSource", "src"));
  
  MarSystem* ffanout = mng.create("Fanout", "ffanout");
  
  MarSystem* stereobranches = mng.create("Parallel", "stereobranches");
  MarSystem* left = mng.create("Series", "left");
  MarSystem* right = mng.create("Series", "right");

  left->addMarSystem(mng.create("Windowing", "hamleft"));
  left->addMarSystem(mng.create("Spectrum", "spkleft"));
  left->addMarSystem(mng.create("PowerSpectrum", "leftpspk"));
  left->addMarSystem(mng.create("MFCC", "leftMFCC"));
  left->addMarSystem(mng.create("TextureStats", "leftTextureStats"));
  
  right->addMarSystem(mng.create("Windowing", "hamright"));
  right->addMarSystem(mng.create("Spectrum", "spkright"));
  right->addMarSystem(mng.create("PowerSpectrum", "rightpspk"));
  right->addMarSystem(mng.create("MFCC", "rightMFCC"));
  right->addMarSystem(mng.create("TextureStats", "rightTextureStats"));

  stereobranches->addMarSystem(left);
  stereobranches->addMarSystem(right);

  MarSystem* secondbranch = mng.create("Series", "secondbranch");
  MarSystem* stereobranches1 = mng.create("Parallel", "stereobranches1");
  MarSystem* left1 = mng.create("Series", "left1");
  MarSystem* right1 = mng.create("Series", "right1");
  
  left1->addMarSystem(mng.create("Windowing", "hamleft1"));
  left1->addMarSystem(mng.create("Spectrum", "spkleft1"));
  right1->addMarSystem(mng.create("Windowing", "hamright1"));
  right1->addMarSystem(mng.create("Spectrum", "spkright1"));

  stereobranches1->addMarSystem(left1);
  stereobranches1->addMarSystem(right1);

  secondbranch->addMarSystem(stereobranches1);
  secondbranch->addMarSystem(mng.create("StereoSpectrum", "sspk"));
  secondbranch->addMarSystem(mng.create("StereoSpectrumFeatures", "sspkf"));
  secondbranch->addMarSystem(mng.create("TextureStats", "texturests"));





  playbacknet->addMarSystem(ffanout);
  ffanout->addMarSystem(stereobranches);
  ffanout->addMarSystem(secondbranch);

  MarSystem* acc = mng.create("Accumulator", "acc");
  acc->addMarSystem(playbacknet);
  
  MarSystem* statistics2 = mng.create("Fanout", "statistics2");
  statistics2->addMarSystem(mng.create("Mean", "mn"));
  statistics2->addMarSystem(mng.create("StandardDeviation", "std"));

  MarSystem* total = mng.create("Series", "total");
  total->addMarSystem(acc);
  total->updctrl("Accumulator/acc/mrs_natural/nTimes", 1000);
  total->addMarSystem(statistics2);

  total->addMarSystem(mng.create("Annotator", "ann"));
  total->addMarSystem(mng.create("WekaSink", "wsink"));

  total->updctrl("WekaSink/wsink/mrs_natural/nLabels", (mrs_natural)cls.size());
  total->updctrl("WekaSink/wsink/mrs_natural/downsample", 1); 
  total->updctrl("WekaSink/wsink/mrs_string/labelNames", classNames);
  total->updctrl("WekaSink/wsink/mrs_string/filename", wekafname); 

  total->updctrl("mrs_natural/inSamples", 1024);

  mrs_bool isEmpty;

  // cout << *total << endl;
  int cj,i;
  for (cj=0; cj < cls.size(); cj++)
    {
      Collection l = cls[cj];
      total->updctrl("Annotator/ann/mrs_natural/label", cj); 
      for (i=0; i < l.size(); i++)
	{
	  total->updctrl("Accumulator/acc/Series/playbacknet/SoundFileSource/src/mrs_string/filename", l.entry(i));	  
	  cout << "Processing" << l.entry(i) << endl;
	  total->tick();	  
	}
    }
  
  
}



void 
bextract_trainStereoMFCC(vector<Collection> cls, string classNames, 
		     string wekafname, mrs_natural memSize)
{
  cout << "STEREO" << endl;
  cout << "classNames = "  << classNames << endl;
  cout << "wekafname = "  << wekafname << endl;
  


  MarSystemManager mng;

  MarSystem* playbacknet = mng.create("Series", "playbacknet");
  playbacknet->addMarSystem(mng.create("SoundFileSource", "src"));
  
  MarSystem* stereobranches = mng.create("Parallel", "stereobranches");
  MarSystem* left = mng.create("Series", "left");
  MarSystem* right = mng.create("Series", "right");

  left->addMarSystem(mng.create("Windowing", "hamleft"));
  left->addMarSystem(mng.create("Spectrum", "spkleft"));
  left->addMarSystem(mng.create("PowerSpectrum", "leftpspk"));
  left->addMarSystem(mng.create("MFCC", "leftMFCC"));
  left->addMarSystem(mng.create("TextureStats", "leftTextureStats"));
  
  right->addMarSystem(mng.create("Windowing", "hamright"));
  right->addMarSystem(mng.create("Spectrum", "spkright"));
  right->addMarSystem(mng.create("PowerSpectrum", "rightpspk"));
  right->addMarSystem(mng.create("MFCC", "rightMFCC"));
  right->addMarSystem(mng.create("TextureStats", "rightTextureStats"));

  stereobranches->addMarSystem(left);
  stereobranches->addMarSystem(right);

  playbacknet->addMarSystem(stereobranches);

  MarSystem* acc = mng.create("Accumulator", "acc");
  acc->addMarSystem(playbacknet);
  
  MarSystem* statistics2 = mng.create("Fanout", "statistics2");
  statistics2->addMarSystem(mng.create("Mean", "mn"));
  statistics2->addMarSystem(mng.create("StandardDeviation", "std"));

  MarSystem* total = mng.create("Series", "total");
  total->addMarSystem(acc);
  total->updctrl("Accumulator/acc/mrs_natural/nTimes", 1000);
  total->addMarSystem(statistics2);

  total->addMarSystem(mng.create("Annotator", "ann"));
  total->addMarSystem(mng.create("WekaSink", "wsink"));

  
  total->updctrl("WekaSink/wsink/mrs_natural/nLabels", (mrs_natural)cls.size());
  total->updctrl("WekaSink/wsink/mrs_natural/downsample", 1); 
  total->updctrl("WekaSink/wsink/mrs_string/labelNames", classNames);
  total->updctrl("WekaSink/wsink/mrs_string/filename", wekafname); 
  
  playbacknet->linkControl("mrs_bool/notEmpty", "SoundFileSource/src/mrs_bool/notEmpty");


  total->updctrl("mrs_natural/inSamples", 1024);

  mrs_bool isEmpty;

  // cout << *total << endl;
  int cj,i;
  for (cj=0; cj < cls.size(); cj++)
    {
      Collection l = cls[cj];
      total->updctrl("Annotator/ann/mrs_natural/label", cj); 
      for (i=0; i < l.size(); i++)
	{
	  total->updctrl("Accumulator/acc/Series/playbacknet/SoundFileSource/src/mrs_string/filename", l.entry(i));	  
	  cout << "Processing" << l.entry(i) << endl;
	  total->tick();	  
	}
    }
  
  
}
		     





void bextract_trainAccumulator(vector<Collection> cls, mrs_natural label, 
			       string pluginName, string classNames, 
			       string wekafname, string filefeaturename, 
			       mrs_natural memSize, string extractorStr,
			       bool withBeatFeatures)
{
  cout << "bextractTrainAccumulator::extractorStr = " << extractorStr << endl;
  if (withBeatFeatures)
    cout << "with beat features" << endl;


  


  MRSDIAG("bextract.cpp - bextract_trainAccumulator");
  mrs_natural i;
  mrs_natural cj;

  Collection linitial = cls[0];
  string sfName = linitial.entry(0);

  if (normopt) 
    cout << "NORMALIZE ENABLED" << endl;

  MarSystemManager mng;  
		
  ////////////////////////////////////////////////
//////////////////////////
  // create the file source
  //////////////////////////////////////////////////////////////////////////
  MarSystem* src = mng.create("SoundFileSource", "src");

  //////////////////////////////////////////////////////////////////////////
  // Feature Extractor
  //////////////////////////////////////////////////////////////////////////
  // create the correct feature extractor using the table of known
  // feature extractors:
  MarSystem* featExtractor = (*featExtractors[extractorStr])();
  featExtractor->updctrl("mrs_natural/WindowSize", winSize);
	
  //////////////////////////////////////////////////////////////////////////
  // Build the overall feature calculation network
  //////////////////////////////////////////////////////////////////////////
  MarSystem* featureNetwork = mng.create("Series", "featureNetwork");
  featureNetwork->addMarSystem(src);
  
  // convert stereo files to mono 
  featureNetwork->addMarSystem(mng.create("Stereo2Mono", "s2m"));
  featureNetwork->addMarSystem(featExtractor);

  //////////////////////////////////////////////////////////////////////////
  // Texture Window Statistics (if any)
  //////////////////////////////////////////////////////////////////////////
  if(memSize != 0)
    {
      featureNetwork->addMarSystem(mng.create("TextureStats", "tStats"));
      featureNetwork->updctrl("TextureStats/tStats/mrs_natural/memSize", memSize);
    }

  //////////////////////////////////////////////////////////////////////////
  // update controls
  //////////////////////////////////////////////////////////////////////////
  featureNetwork->updctrl("SoundFileSource/src/mrs_string/filename", sfName);
  featureNetwork->updctrl("mrs_natural/inSamples", MRS_DEFAULT_SLICE_NSAMPLES);

  //////////////////////////////////////////////////////////////////////////
  // accumulate feature vectors over 30 seconds 
  //////////////////////////////////////////////////////////////////////////
  MarSystem* acc = mng.create("Accumulator", "acc");
  acc->updctrl("mrs_natural/nTimes", accSize_);
	
  //////////////////////////////////////////////////////////////////////////
  // add network to accumulator
  //////////////////////////////////////////////////////////////////////////
  acc->addMarSystem(featureNetwork->clone());

  //////////////////////////////////////////////////////////////////////////
  // WEKA output
  //////////////////////////////////////////////////////////////////////////
  MarSystem* wsink = mng.create("WekaSink", "wsink");

  //////////////////////////////////////////////////////////////////////////
  // Annotator
  //////////////////////////////////////////////////////////////////////////
  MarSystem* annotator = mng.create("Annotator", "annotator");

  //////////////////////////////////////////////////////////////////////////
  // 30-second statistics 
  //////////////////////////////////////////////////////////////////////////
  MarSystem* statistics = mng.create("Fanout", "statistics2");
  statistics->addMarSystem(mng.create("Mean", "mn"));
  statistics->addMarSystem(mng.create("StandardDeviation", "std"));
	
  //////////////////////////////////////////////////////////////////////////
  // Final network compute 30-second statistics 
  //////////////////////////////////////////////////////////////////////////
  MarSystem* total = mng.create("Series", "total");
  total->addMarSystem(acc);
  total->addMarSystem(statistics);

  // get parameters
  mrs_real srate = samplingRate_;


  total->updctrl("mrs_natural/inSamples", winSize);

  total->updctrl("Accumulator/acc/Series/featureNetwork/" + src->getType() + "/src/mrs_natural/pos", offset);      

  // Calculate duration, offset parameters if necessary 
  offset = (mrs_natural) (start * samplingRate_ );
  duration = (mrs_natural) (length * samplingRate_);

  //////////////////////////////////////////////////////////////////////////
  // main loop for extracting the features 
  //////////////////////////////////////////////////////////////////////////
  mrs_natural wc = 0;
  mrs_natural samplesPlayed =0;
  string className = "";
  realvec beatfeatures;
  beatfeatures.create((mrs_natural)8,(mrs_natural)1);
  realvec estimate;
  estimate.create((mrs_natural)8,(mrs_natural)1);
  realvec in;
  realvec timbreres;
  realvec fullres;
  realvec afullres;

  in.create(total->getctrl("mrs_natural/inObservations")->toNatural(), 
	    total->getctrl("mrs_natural/inSamples")->toNatural());
  timbreres.create(total->getctrl("mrs_natural/onObservations")->toNatural(), 
		   total->getctrl("mrs_natural/onSamples")->toNatural());

  if (withBeatFeatures)
    {
      fullres.create(total->getctrl("mrs_natural/onObservations")->toNatural() + 8, 
		     total->getctrl("mrs_natural/onSamples")->toNatural());
      afullres.create(total->getctrl("mrs_natural/onObservations")->toNatural() + 8 + 1, 
		      total->getctrl("mrs_natural/onSamples")->toNatural());
      annotator->updctrl("mrs_natural/inObservations", total->getctrl("mrs_natural/onObservations")->toNatural()+8);      
    }
  else
    {
      fullres.create(total->getctrl("mrs_natural/onObservations")->toNatural(), 
		     total->getctrl("mrs_natural/onSamples")->toNatural());

      afullres.create(total->getctrl("mrs_natural/onObservations")->toNatural() + 1,
		      total->getctrl("mrs_natural/onSamples")->toNatural());  
      annotator->updctrl("mrs_natural/inObservations", total->getctrl("mrs_natural/onObservations")->toNatural());      
    }

  annotator->updctrl("mrs_natural/inSamples", total->getctrl("mrs_natural/onSamples"));
  annotator->updctrl("mrs_real/israte", total->getctrl("mrs_real/israte"));

  wsink->updctrl("mrs_natural/inSamples", annotator->getctrl("mrs_natural/onSamples"));
  wsink->updctrl("mrs_natural/inObservations", annotator->getctrl("mrs_natural/onObservations")->toNatural());
  wsink->updctrl("mrs_real/israte", annotator->getctrl("mrs_real/israte"));
  wsink->updctrl("mrs_string/labelNames",classNames);

  mrs_natural timbreSize = total->getctrl("mrs_natural/onObservations")->toNatural();
  mrs_natural beatSize = 8;

  wsink->updctrl("mrs_natural/nLabels", (mrs_natural)cls.size());  

  MarSystem *total1 = NULL;

  if (withBeatFeatures) 
    {
      total1 = createBEATextrator();
      annotator->updctrl("mrs_string/inObsNames", total->getctrl("mrs_string/onObsNames")->toString() + total1->getctrl("mrs_string/onObsNames")->toString());
    }
  else
    {
      annotator->updctrl("mrs_string/inObsNames", total->getctrl("mrs_string/onObsNames"));  
    }

  wsink->updctrl("mrs_string/inObsNames", annotator->getctrl("mrs_string/onObsNames"));

  realvec iwin;
  if (wekafname == EMPTYSTRING) 
    wsink->updctrl("mrs_string/filename", "weka.arff");
  else 
    wsink->updctrl("mrs_string/filename", wekafname);





  //iterate over collections
  for (cj=0; cj < (mrs_natural)cls.size(); cj++)
    {
      Collection l = cls[cj];
      for (i=0; i < l.size(); i++)//iterate over collection files
	{
	  // cout << beatfeatures << endl;
	  total->updctrl("Accumulator/acc/Series/featureNetwork/SoundFileSource/src/mrs_string/filename", l.entry(i));
	  if (withBeatFeatures) 
	    {
	      srate = total->getctrl("Accumulator/acc/Series/featureNetwork/SoundFileSource/src/mrs_real/osrate")->toReal();
	      iwin.create((mrs_natural)1, (mrs_natural)(((srate / 22050.0) * 2 * 65536) / 16)); // [!] hardcoded!
	      tempo_histoSumBands(total1, l.entry(i), beatfeatures, 
				  iwin, estimate);
	    }
	  total->updctrl("Accumulator/acc/Series/featureNetwork/SoundFileSource/src/mrs_natural/pos", offset);
	  wc = 0;  	  
	  samplesPlayed = 0;
	  // total->updctrl("WekaSink/wsink/mrs_natural/label", cj);
	  annotator->updctrl("mrs_natural/label", cj);
	  // wsink->updctrl("mrs_natural/label", cj);
	  total->process(in, timbreres);

	  // concatenate timbre and beat vectors 
	  for (int t=0; t < timbreSize; t++)
	    fullres(t,0) = timbreres(t,0);

	  if (withBeatFeatures)
	    {
	      for (int t=0; t < beatSize; t++)
		fullres(t+timbreSize, 0) = beatfeatures(t,0);
	    }

	  annotator->process(fullres, afullres);
	  wsink->process(afullres, afullres);

	  cerr << "Processed " << l.entry(i) << endl;
	}
    }


  if (pluginName == EMPTYSTRING) // output to stdout 
    cout << (*total) << endl;      
  else 
    {
      ofstream oss(pluginName.c_str());
      oss << (*total) << endl;
    }
  
  delete featureNetwork;
}

// train with multiple feature vectors/file 
void bextract_train(vector<Collection> cls, 
		    mrs_natural label, 
		    string pluginName, string classNames, 
		    string wekafname,  mrs_natural memSize, 
		    string extractorStr,
		    string classifierName)
{
  MRSDIAG("bextract.cpp - bextract_train");

  MarSystemManager mng;

  vector<TimeLine> timeLines;

  if (classifierName == EMPTYSTRING) 
    classifierName = DEFAULT_CLASSIFIER;

  if (extractorStr == EMPTYSTRING) 
    extractorStr = DEFAULT_EXTRACTOR; 

 
  //////////////////////////////////////////////////////////////////////////
  // Find proper sound file format and create SignalSource
  //////////////////////////////////////////////////////////////////////////
  Collection linitial = cls[0];
  string sfName = linitial.entry(0);
  MarSystem *src = mng.create("SoundFileSource", "src");
  src->updctrl("mrs_string/filename", sfName);
  // src->updctrl("mrs_natural/inSamples", MRS_DEFAULT_SLICE_NSAMPLES);
  // src->updctrl("mrs_natural/inSamples", 2048);

  // Calculate duration, offset parameters if necessary 
  if (start > 0.0) 
    offset = (mrs_natural) (start 
			    * src->getctrl("mrs_real/israte")->toReal() 
			    * src->getctrl("mrs_natural/onObservations")->toNatural());

  if (length != 30.0) 
    duration = (mrs_natural) (length 
			      * src->getctrl("mrs_real/israte")->toReal() 
			      * src->getctrl("mrs_natural/onObservations")->toNatural());

  //////////////////////////////////////////////////////////////////////////
  // Feature Extractor
  //////////////////////////////////////////////////////////////////////////
  // create the correct feature extractor using the table of known
  // feature extractors:
  MarSystem* featExtractor = (*featExtractors[extractorStr])();
	
  featExtractor->updctrl("mrs_natural/WindowSize", winSize);

  //////////////////////////////////////////////////////////////////////////
  // Build the overall feature calculation network 
  //////////////////////////////////////////////////////////////////////////
  MarSystem* featureNetwork = mng.create("Series", "featureNetwork");
  featureNetwork->addMarSystem(src);

  if (pluginName != EMPTYSTRING)
    {
      // create audio sink and mute it 
      // it is stored in the output plugin 
      // which can be used for real-time classification 
      MarSystem* dest = mng.create("AudioSink", "dest");
      dest->updctrl("mrs_bool/mute", true);
      featureNetwork->addMarSystem(dest);
    }
	
  //add the feature extraction network
  featureNetwork->addMarSystem(featExtractor);//mng.create("FeatureExtractor", "featExtractor"));

  //////////////////////////////////////////////////////////////////////////
  //texture window statistics (optional)
  //////////////////////////////////////////////////////////////////////////
  if(memSize != 0)
    {
      featureNetwork->addMarSystem(mng.create("TextureStats", "tStats"));
      featureNetwork->updctrl("TextureStats/tStats/mrs_natural/memSize", memSize);
    }
	
  //////////////////////////////////////////////////////////////////////////
  // update controls I
  //////////////////////////////////////////////////////////////////////////
  // src has to be configured with hopSize frame length in case a ShiftInput
  // is used in the feature extraction network
  featureNetwork->updctrl("mrs_natural/inSamples", hopSize);
  featureNetwork->updctrl(src->getType() + "/src/mrs_natural/pos", offset);      
	
  //////////////////////////////////////////////////////////////////////////
  // add the Annotator
  //////////////////////////////////////////////////////////////////////////
  featureNetwork->addMarSystem(mng.create("Annotator", "annotator"));
	
  //////////////////////////////////////////////////////////////////////////
  // add WEKA sink
  //////////////////////////////////////////////////////////////////////////
  if (wekafname != EMPTYSTRING)
    featureNetwork->addMarSystem(mng.create("WekaSink", "wsink"));
	
  //////////////////////////////////////////////////////////////////////////
  // add classifier and confidence majority calculation 
  //////////////////////////////////////////////////////////////////////////
  cout << "classifierName = " << classifierName << endl;
  if (classifierName == "GS")
    featureNetwork->addMarSystem(mng.create("GaussianClassifier", "gaussian"));
  else if (classifierName == "ZeroR")
    featureNetwork->addMarSystem(mng.create("ZeroRClassifier", "zeror"));
  else if (classifierName == "KNN")
    featureNetwork->addMarSystem(mng.create("KNNClassifier", "knn"));
  else
    {
      cerr << "Unsuported classifier : " << classifierName << endl;
      return;
    }

  featureNetwork->addMarSystem(mng.create("Confidence", "confidence"));

  // FileName Sfname(sfName);
      // Sfname.path()+
   //   featureNetwork->updctrl("Confidence/confidence/mrs_string/fileName", Sfname.nameNoExt()); 
 

      
  //////////////////////////////////////////////////////////////////////////
  // link controls
  //////////////////////////////////////////////////////////////////////////
  featureNetwork->linkctrl("mrs_string/filename", "SoundFileSource/src/mrs_string/filename");
  featureNetwork->linkctrl("SoundFileSource/src/mrs_string/currentlyPlaying", "Confidence/confidence/mrs_string/fileName");
  //  featureNetwork->linkctrl("SoundFileSource/src/mrs_string/filename", "Confidence/confidence/mrs_string/fileName");

  featureNetwork->linkctrl("mrs_real/israte", "SoundFileSource/src/mrs_real/israte");
  featureNetwork->linkctrl("mrs_natural/pos", "SoundFileSource/src/mrs_natural/pos");
  
  featureNetwork->linkctrl("mrs_bool/notEmpty", "SoundFileSource/src/mrs_bool/notEmpty");
  featureNetwork->linkctrl("mrs_bool/initAudio", "AudioSink/dest/mrs_bool/initAudio");
  
  MarControlPtr ctrl_filename_ = featureNetwork->getctrl("SoundFileSource/src/mrs_string/filename");
  MarControlPtr ctrl_notEmpty_ = featureNetwork->getctrl("SoundFileSource/src/mrs_bool/notEmpty");
  



  //////////////////////////////////////////////////////////////////////////
  // main loop for extracting features 
  //////////////////////////////////////////////////////////////////////////
  //***********************************
  // if no timelines are being used...
  //***********************************
  if(!tline)
    {
      mrs_natural cj;
      mrs_natural wc = 0;
      mrs_natural samplesPlayed =0;
      mrs_natural onSamples = featureNetwork->getctrl("mrs_natural/onSamples")->toNatural();

      if (classifierName == "GS")
	featureNetwork->updctrl("GaussianClassifier/gaussian/mrs_natural/nLabels", (mrs_natural)cls.size());
      else if (classifierName == "ZeroR")
	featureNetwork->updctrl("ZeroRClassifier/zeror/mrs_natural/nLabels", (mrs_natural)cls.size());
      else if (classifierName == "KNN")
	featureNetwork->updctrl("KNNClassifier/knn/mrs_natural/nLabels", (mrs_natural)cls.size());

      //configure Confidence
      featureNetwork->updctrl("Confidence/confidence/mrs_natural/nLabels", (int)cls.size());
      featureNetwork->updctrl("Confidence/confidence/mrs_bool/mute", true);
      featureNetwork->updctrl("Confidence/confidence/mrs_string/labelNames",classNames);
      featureNetwork->updctrl("Confidence/confidence/mrs_bool/print",true);

      //configure WEKA sink
      if (wekafname != EMPTYSTRING)
	featureNetwork->updctrl("WekaSink/wsink/mrs_string/labelNames",classNames);

      //iterate over collections (i.e. classes)
      for (cj=0; cj < (mrs_natural)cls.size(); cj++)
	{
	  Collection l = cls[cj];
	  featureNetwork->updctrl("Annotator/annotator/mrs_natural/label", cj);

	  if (wekafname != EMPTYSTRING)
	    {
	      featureNetwork->updctrl("WekaSink/wsink/mrs_natural/nLabels", (mrs_natural)cls.size());
	      featureNetwork->updctrl("WekaSink/wsink/mrs_natural/downsample", 40);
	      featureNetwork->updctrl("WekaSink/wsink/mrs_string/filename", wekafname);  			
	    }

	  cout << "Class " << cj << " is " << l.name() << endl;

	  //reset texture analysis stats between (i.e. classes)
	  if(memSize != 0)
	    featureNetwork->updctrl("TextureStats/tStats/mrs_bool/reset", true);

	  //iterate over audio files (in each collection) and extract features
	  for (mrs_natural i=0; i < l.size(); i++)
	    {
	      //  featureNetwork->updctrl("SoundFileSource/src/mrs_string/filename", l.entry(i));
	      featureNetwork->updctrl(ctrl_filename_, l.entry(i));
	      wc = 0;  	  
	      samplesPlayed = 0;
	      while (ctrl_notEmpty_->to<mrs_bool>() && (duration > samplesPlayed))
		{
		  featureNetwork->tick();
		  wc++;
		  samplesPlayed = wc * onSamples;
		}
	      featureNetwork->tick();
	      cerr << "Processed " << l.entry(i) << endl;
	    }
	}
    }
  //**********************
  // if using timelines
  //**********************
  else 	
    {
      TimeLine tline;
      mrs_natural numClasses;
      //1st collection is the audio file collection
      Collection audioColl = cls[0];
      //2nd collection  is a collection of timeline files
      //(should be in the same order as the audio files)
      Collection tlColl = cls[1];
		
      // for each audiofile/timeline pair in collection
      // (and ignore all audio files that have no timeline file)
      // run feature extraction and train classifiers
      for(mrs_natural i=0; i <(mrs_natural)tlColl.size(); ++i)
	{
	  //load Audio file from the collection
	  featureNetwork->updctrl("SoundFileSource/src/mrs_string/filename", audioColl.entry(i));
			
	  //load timeline for i-th audio file
	  tline.load(tlColl.entry(i));
			
	  //get number of classes in the timeline
	  numClasses = (mrs_natural)tline.numClasses();
			
	  // create a label for each class
	  classNames = "";
	  ostringstream sstr;
	  for(mrs_natural c=0; c < numClasses; c++)
	    sstr << "class_" << c << ",";
	  classNames = sstr.str();

	  //configure classifiers
	  if (classifierName == "GS")
	    featureNetwork->updctrl("GaussianClassifier/gaussian/mrs_natural/nLabels", numClasses);
	  else if (classifierName == "ZeroR")
	    featureNetwork->updctrl("ZeroRClassifier/zeror/mrs_natural/nLabels", numClasses);
	  else if (classifierName == "KNN")
	    featureNetwork->updctrl("KNNClassifier/knn/mrs_natural/nLabels", numClasses);

	  //configure Confidence
	  featureNetwork->updctrl("Confidence/confidence/mrs_natural/nLabels", numClasses);
	  featureNetwork->updctrl("Confidence/confidence/mrs_bool/mute", true);
	  featureNetwork->updctrl("Confidence/confidence/mrs_string/labelNames",classNames);
	  featureNetwork->updctrl("Confidence/confidence/mrs_bool/print",true); 

	  //configure WEKA sink
	  if (wekafname != EMPTYSTRING)
	    {
	      featureNetwork->updctrl("WekaSink/wsink/mrs_natural/nLabels", numClasses);
	      featureNetwork->updctrl("WekaSink/wsink/mrs_string/labelNames",classNames);
	      featureNetwork->updctrl("WekaSink/wsink/mrs_natural/downsample", 40);
	      string name = audioColl.entry(i).substr(0, audioColl.entry(i).length()-4) + "_" + extractorStr + ".arff";
	      featureNetwork->updctrl("WekaSink/wsink/mrs_string/filename", name);
	    }

	  cout << endl;
	  cout << "**************************************" << endl;
	  cout << "AudioFile: " << audioColl.entry(i) << endl;
	  cout << "TimeLine : " << tlColl.entry(i) << endl;
	  cout << "**************************************" << endl;

	  //iterate over timeline regions
	  for (mrs_natural r = 0; r < tline.numRegions(); r++)
	    {
	      cout << "-----------------------------------------------" << endl;
	      cout << "Region " << r+1 << "/" << tline.numRegions() << endl;
	      cout << "Region start   = " << tline.start(r) << endl;
	      cout << "Region classID = " << tline.getRClassId(r) << endl; 
	      cout << "Region end     = " << tline.end(r) << endl;

	      // set current region class in Annotator
	      featureNetwork->updctrl("Annotator/annotator/mrs_natural/label", tline.getRClassId(r));

	      // set current region class in WEKA sink 
	      if (wekafname != EMPTYSTRING)
		{
		  featureNetwork->updctrl("WekaSink/wsink/mrs_natural/label", tline.getRClassId(r)); //[?]
		}

	      // reset texture analysis stats between regions
	      if(memSize != 0)
		featureNetwork->updctrl("TextureStats/tStats/mrs_bool/reset", true);

	      //define audio region boundaries
	      mrs_natural start = (mrs_natural)(tline.start(r) * tline.lineSize_); //region start sample
	      mrs_natural end = (mrs_natural)(tline.end(r) * tline.lineSize_); //region end sample

	      mrs_natural fileSize = featureNetwork->getctrl(src->getType() + "/src/mrs_natural/size")->to<mrs_natural>();
	      if(end > fileSize)
		{
		  end = fileSize;
		  cout << "WARNING:" << endl;
		  cout << "Region end sample behind EOF!" << endl;
		  cout << "Setting region end sample to EOF." << endl;
		}

	      featureNetwork->updctrl(src->getType() + "/src/mrs_natural/pos", start);
	      //featureNetwork->updctrl("mrs_natural/inSamples", hopSize); //[?]
	      //featureNetwork->updctrl("mrs_natural/inSamples", tline.lineSize_);//[?]
				
	      //iterate over audio region
	      //(using the winSize and hopSize defined by the user)
	      mrs_natural numWindows = 0;
	      while(featureNetwork->getctrl(src->getType() + "/src/mrs_natural/pos")->to<mrs_natural>() + winSize <= end)
		{
		  //cout << "pos = " << featureNetwork->getctrl(src->getType() + "/src/mrs_natural/pos")->to<mrs_natural>() << endl;
		  featureNetwork->tick();
		  numWindows++;
		  cout << '.';
		}
	      cout << endl;

	      mrs_natural lastpos = featureNetwork->getctrl(src->getType() + "/src/mrs_natural/pos")->to<mrs_natural>();
	      if((end - lastpos) > 0)
		{
		  cout << "Dropped last frame: " << endl;
		  cout << "Dropped " << end - lastpos << " samples from current segment." << endl;
		  cout << "Dropped " <<(lastpos + winSize) - end << " samples from next segment." << endl;
		}
				
	      cout << "Processed " << numWindows << " frames!"  << endl;
	    }
	}
    }

  //////////////////////////////////////////////////////////////////////////
  // update and train classifier after feature extraction is complete
  //////////////////////////////////////////////////////////////////////////
  if (classifierName == "GS")
    featureNetwork->updctrl("GaussianClassifier/gaussian/mrs_bool/done",true);
  else if (classifierName == "ZeroR")  
    featureNetwork->updctrl("ZeroRClassifier/zeror/mrs_bool/done",true);
  else if (classifierName == "KNN")  
    featureNetwork->updctrl("KNNClassifier/knn/mrs_bool/done",true);
	
  // train classifier
  featureNetwork->tick();		

  //////////////////////////////////////////////////////////////////////////
  // prepare network for classification
  //////////////////////////////////////////////////////////////////////////
  if (classifierName == "GS")
    {
      featureNetwork->updctrl("GaussianClassifier/gaussian/mrs_bool/done",false);  
      featureNetwork->updctrl("GaussianClassifier/gaussian/mrs_string/mode","predict"); 
    }
  else if (classifierName == "ZeroR")  
    {
      featureNetwork->updctrl("ZeroRClassifier/zeror/mrs_bool/done",false);  
      featureNetwork->updctrl("ZeroRClassifier/zeror/mrs_string/mode","predict") ;
    }
  else if (classifierName == "KNN")  
    {
      featureNetwork->updctrl("KNNClassifier/knn/mrs_bool/done",false);  
      featureNetwork->updctrl("KNNClassifier/knn/mrs_string/mode","predict");
      featureNetwork->updctrl("KNNClassifier/knn/mrs_natural/k",3); //[!] hardcoded!!!
    }  

  if (pluginName != EMPTYSTRING && !pluginMute) 
    {
      featureNetwork->updctrl("AudioSink/dest/mrs_bool/mute", false);
      featureNetwork->updctrl("AudioSink/dest/mrs_bool/initAudio", true);//[!][?] this still does not solves the problem of sfplugin being unable to play audio... 
    }
	
  if (wekafname != EMPTYSTRING)
    featureNetwork->updctrl("WekaSink/wsink/mrs_bool/mute", true);  
	
  featureNetwork->updctrl("Confidence/confidence/mrs_bool/mute", false);

  //////////////////////////////////////////////////////////////////////////
  // output trained classifier models
  //////////////////////////////////////////////////////////////////////////
  if (pluginName == EMPTYSTRING) // output to stdout 
    cout << (*featureNetwork) << endl;      
  else // save to .mpl file
    {
      ofstream oss(pluginName.c_str());
      oss << (*featureNetwork) << endl;
    }

  delete featureNetwork;
}

// train with multiple feature vectors/file 
void bextract_train_rmsilence(vector<Collection> cls, mrs_natural label, 
			      string pluginName, string classNames, 
			      string wekafname,  mrs_natural memSize, 
			      string extractorStr,
			      string classifierName)
{
  if (classifierName == EMPTYSTRING) 
    classifierName = "SMO";

  MRSDIAG("bextract.cpp - bextract_train");
  mrs_natural i;
  mrs_natural cj;

  Collection linitial = cls[0];
  string sfName = linitial.entry(0);

  // default 
  if (extractorStr == EMPTYSTRING) 
    extractorStr = "STFT";

  MarSystemManager mng;

  // Find proper soundfile format and create SignalSource 
  MarSystem *srm = mng.create("SilenceRemove", "srm");
  MarSystem *src = mng.create("SoundFileSource", "src");
  src->updctrl("mrs_string/filename", sfName);

  srm->addMarSystem(src);

  // src->updctrl("mrs_natural/inSamples", MRS_DEFAULT_SLICE_NSAMPLES);
  // srm->updctrl("mrs_natural/inSamples", 2048);
  // srm->updctrl("mrs_natural/inSamples", 2048);

  // Calculate duration, offest parameters if necessary 
  if (start > 0.0f) 
    offset = (mrs_natural) (start 
			    * src->getctrl("mrs_real/israte")->toReal());
  

  if (length != 30.0f) 
    duration = (mrs_natural) (length 
			      * src->getctrl("mrs_real/israte")->toReal());
  

  // create audio sink and mute it 
  // it is stored in the output plugin 
  // which can be used for real-time classification 
  MarSystem* dest=NULL;

  if (pluginName != EMPTYSTRING) // output to stdout 
    {
      dest = mng.create("AudioSink", "dest");
      dest->updctrl("mrs_bool/mute", true);
    }

  // Calculate windowed power spectrum and then 
  // calculate specific feature sets 
  MarSystem* spectralShape = mng.create("Series", "spectralShape");
  spectralShape->addMarSystem(mng.create("Windowing", "hamming"));
  spectralShape->addMarSystem(mng.create("Spectrum","spk"));
  spectralShape->updctrl("Spectrum/spk/mrs_real/cutoff", 1.0);
  spectralShape->addMarSystem(mng.create("PowerSpectrum", "pspk"));
  spectralShape->updctrl("PowerSpectrum/pspk/mrs_string/spectrumType","power");  

  // Spectrum Shape descriptors
  MarSystem *spectrumFeatures = mng.create("Fanout",  "spectrumFeatures");
  if (extractorStr == "STFT") 
    {
      spectrumFeatures->addMarSystem(mng.create("Centroid", "cntrd"));
      spectrumFeatures->addMarSystem(mng.create("Rolloff", "rlf"));      
      spectrumFeatures->addMarSystem(mng.create("Flux", "flux"));
    }
  else if (extractorStr == "STFTMFCC")
    {
      spectrumFeatures->addMarSystem(mng.create("Centroid", "cntrd"));
      spectrumFeatures->addMarSystem(mng.create("Rolloff", "rlf"));      
      spectrumFeatures->addMarSystem(mng.create("Flux", "flux"));
      spectrumFeatures->addMarSystem(mng.create("MFCC", "mfcc"));
    }
  else if (extractorStr == "MFCC")
    spectrumFeatures->addMarSystem(mng.create("MFCC", "mfcc"));
  else if (extractorStr == "SCF")
    spectrumFeatures->addMarSystem(mng.create("SCF", "scf"));
  else if (extractorStr == "SFM")
    spectrumFeatures->addMarSystem(mng.create("SFM", "sfm"));
  else 
    {
      cerr << "Extractor " << extractorStr << " is not supported " << endl;
      return;
    }


  // add the feature to spectral shape
  spectralShape->addMarSystem(spectrumFeatures);

  //  add time-domain zerocrossings
  MarSystem* features = mng.create("Fanout", "features");
  features->addMarSystem(spectralShape);

  if (extractorStr == "STFT")
    features->addMarSystem(mng.create("ZeroCrossings", "zcrs"));      

  // Means and standard deviation (statistics) for texture analysis 
  MarSystem* statistics = mng.create("Fanout", "statistics");
  statistics->addMarSystem(mng.create("Mean", "mn"));
  statistics->addMarSystem(mng.create("StandardDeviation", "std"));

  // Weka output 
  MarSystem* wsink = mng.create("WekaSink", "wsink");

  // Build the overall feature calculation network   
  MarSystem* featureNetwork = mng.create("Series", "featureNetwork");
  featureNetwork->addMarSystem(srm->clone());

  if (pluginName != EMPTYSTRING) // don't put audio object
    featureNetwork->addMarSystem(dest);

  cout << "featureNetwork = " << (*featureNetwork) << endl;

  featureNetwork->addMarSystem(features);
  featureNetwork->addMarSystem(mng.create("Memory", "memory"));
  featureNetwork->addMarSystem(statistics);  
  if (classifierName == "SMO")
    featureNetwork->addMarSystem(mng.create("NormMaxMin", "norm"));

  // update controls I
  featureNetwork->updctrl("Memory/memory/mrs_natural/memSize", memSize);
  // featureNetwork->updctrl("mrs_natural/inSamples", 
  // MRS_DEFAULT_SLICE_NSAMPLES);

  featureNetwork->updctrl("mrs_natural/inSamples", winSize);
  featureNetwork->updctrl("SilenceRemove/srm/" + src->getType() + "/src/mrs_natural/pos", offset);      
  featureNetwork->addMarSystem(mng.create("Annotator", "annotator"));
  featureNetwork->addMarSystem(wsink->clone());

  // add classifier and confidence majority calculation 
  if (classifierName == "GS")
    featureNetwork->addMarSystem(mng.create("GaussianClassifier", "gaussian"));
  else if (classifierName == "ZeroR")
    featureNetwork->addMarSystem(mng.create("ZeroRClassifier", "zeror"));
  else if (classifierName == "KNN")
    featureNetwork->addMarSystem(mng.create("KNNClassifier", "knn"));
  else if (classifierName == "SMO") 
    featureNetwork->addMarSystem(mng.create("SMO", "smo"));
  else
    {
      cerr << "Unsuported classifier : " << classifierName << endl;
      return;
    }

  featureNetwork->addMarSystem(mng.create("Confidence", "confidence"));


  // update controls II 
  if (classifierName == "GS")
    featureNetwork->updctrl("GaussianClassifier/gaussian/mrs_natural/nLabels", (mrs_natural)cls.size());
  else if (classifierName == "ZeroR")
    featureNetwork->updctrl("ZeroRClassifier/zeror/mrs_natural/nLabels", (mrs_natural)cls.size());
  else if (classifierName == "KNN")
    featureNetwork->updctrl("KNNClassifier/knn/mrs_natural/nLabels", (mrs_natural)cls.size());
  else if (classifierName == "SMO")
    featureNetwork->updctrl("SMO/smo/mrs_natural/nLabels", (mrs_natural)cls.size());

  featureNetwork->updctrl("Confidence/confidence/mrs_bool/mute", true);
  featureNetwork->updctrl("Confidence/confidence/mrs_string/labelNames",classNames);
  featureNetwork->updctrl("WekaSink/wsink/mrs_string/labelNames",classNames);



  // link controls
  featureNetwork->linkctrl("mrs_string/filename", 
			   "SilenceRemove/srm/SoundFileSource/src/mrs_string/filename");

  featureNetwork->linkctrl("mrs_real/israte", 
			   "SilenceRemove/srm/SoundFileSource/src/mrs_real/israte");
  featureNetwork->linkctrl("mrs_natural/pos", 
			   "SilenceRemove/srm/SoundFileSource/src/mrs_natural/pos");



  featureNetwork->linkctrl("mrs_bool/notEmpty", 
			   "SilenceRemove/srm/SoundFileSource/src/mrs_bool/notEmpty");

  mrs_natural wc = 0;
  mrs_natural samplesPlayed =0;
  mrs_natural onSamples = featureNetwork->getctrl("mrs_natural/onSamples")->toNatural();

  // main loop for extracting the features 
  featureNetwork->updctrl("Confidence/confidence/mrs_natural/nLabels", (int)cls.size());
  string className = "";

  MarControlPtr donePtr = featureNetwork->getctrl("SilenceRemove/srm/SoundFileSource/src/mrs_bool/notEmpty");

  MarControlPtr memResetPtr = featureNetwork->getctrl("Memory/memory/mrs_bool/reset");
  MarControlPtr fnamePtr = featureNetwork->getctrl("SilenceRemove/srm/SoundFileSource/src/mrs_string/filename");
  

  MarControlPtr annLabelPtr = featureNetwork->getctrl("Annotator/annotator/mrs_natural/label");
  MarControlPtr nlabelsPtr = featureNetwork->getctrl("WekaSink/wsink/mrs_natural/nLabels");
  MarControlPtr wekaDownsamplePtr = featureNetwork->getctrl("WekaSink/wsink/mrs_natural/downsample");
  
  MarControlPtr wekaFnamePtr = featureNetwork->getctrl("WekaSink/wsink/mrs_string/filename");
  

  for (cj=0; cj < (mrs_natural)cls.size(); cj++)
    {
      Collection l = cls[cj];
      featureNetwork->setctrl(annLabelPtr, cj);
      featureNetwork->setctrl(nlabelsPtr, (mrs_natural)cls.size());
      featureNetwork->setctrl(wekaDownsamplePtr, 40);
      if (wekafname == EMPTYSTRING) 
	featureNetwork->updctrl(wekaFnamePtr, "weka.arff");
      else 
	featureNetwork->updctrl(wekaFnamePtr, wekafname);  
      // featureNetwork->updctrl("WekaSink/wsink/mrs_natural/label", cj);

      cout << "Class " << cj << " is " << l.name() << endl;

      featureNetwork->setctrl(memResetPtr, true);


      for (i=0; i < l.size(); i++)
	{
	  featureNetwork->setctrl(memResetPtr, true);
	  featureNetwork->updctrl(fnamePtr, l.entry(i));
	  wc = 0;  	  
	  samplesPlayed = 0;



	  while ((donePtr->isTrue()) && (duration > samplesPlayed))
	    {
	      featureNetwork->tick();
	      wc++;
	      samplesPlayed = wc * onSamples;
	    }
	  featureNetwork->tick();
	  cerr << "Processed " << l.entry(i) << endl;
	}
    }

  if (classifierName == "GS")
    featureNetwork->updctrl("GaussianClassifier/gaussian/mrs_bool/done",true);
  else if (classifierName == "ZeroR")  
    featureNetwork->updctrl("ZeroRClassifier/zeror/mrs_bool/done",true);
  else if (classifierName == "KNN")  
    featureNetwork->updctrl("KNNClassifier/knn/mrs_bool/done",true);
  else if (classifierName == "SMO")  
    featureNetwork->updctrl("SMO/smo/mrs_bool/done",true);

  if (classifierName == "SMO")
    featureNetwork->updctrl("NormMaxMin/norm/mrs_bool/train", false);
  featureNetwork->tick();		// train classifier

  // prepare network for classification
  if (classifierName == "GS")
    {
      featureNetwork->updctrl("GaussianClassifier/gaussian/mrs_bool/done",false);  
      featureNetwork->updctrl("GaussianClassifier/gaussian/mrs_string/mode","predict"); 
    }
  else if (classifierName == "ZeroR")  
    {
      featureNetwork->updctrl("ZeroRClassifier/zeror/mrs_bool/done",false);  
      featureNetwork->updctrl("ZeroRClassifier/zeror/mrs_string/mode","predict") ;
    }
  else if (classifierName == "KNN")  
    {
      featureNetwork->updctrl("KNNClassifier/knn/mrs_bool/done",false);  
      featureNetwork->updctrl("KNNClassifier/knn/mrs_string/mode","predict");
      featureNetwork->updctrl("KNNClassifier/knn/mrs_natural/k",3);
    }  
  else if (classifierName == "SMO")  
    {
      featureNetwork->updctrl("SMO/smo/mrs_bool/done",false);  
      featureNetwork->updctrl("SMO/smo/mrs_string/mode","predict");
    }  

  /* if (pluginName != EMPTYSTRING) 
     {
     featureNetwork->updctrl("AudioSink/dest/mrs_bool/mute", false);
     featureNetwork->updctrl("AudioSink/dest/mrs_bool/init", false);
     }
  */ 

  featureNetwork->updctrl("Confidence/confidence/mrs_bool/mute", false);
  featureNetwork->updctrl("mrs_string/filename", "defaultfile");
  featureNetwork->updctrl("WekaSink/wsink/mrs_bool/mute", true);

  if (pluginName == EMPTYSTRING) // output to stdout 
    cout << (*featureNetwork) << endl;      
  else 
    {
      ofstream oss(pluginName.c_str());
      oss << (*featureNetwork) << endl;
    }

  delete featureNetwork;
}

void
readCollection(Collection& l, string name)
{
  MRSDIAG("sfplay.cpp - readCollection");
  ifstream from1(name.c_str());
  mrs_natural attempts  =0;

  MRSDIAG("Trying current working directory: " + name);
  if (from1.good() == false)
    {
      attempts++;
    }
  else
    {
      from1 >> l;
      l.setName(name.substr(0, name.rfind(".", name.length())));
    }

  if (attempts == 1) 
    {
      string warn;
      warn += "Problem reading collection ";
      warn += name; 
      warn += " - tried both default mf directory and current working directory";
      MRSWARN(warn);
      exit(1);
    }
} 

void 
initOptions()
{
  cmd_options.addBoolOption("help", "h", false);
  cmd_options.addBoolOption("usage", "u", false);
  cmd_options.addBoolOption("verbose", "v", false);
  cmd_options.addBoolOption("normalize", "n", false);
  cmd_options.addRealOption("start", "s", 0);
  cmd_options.addRealOption("length", "l", 1000.0f);
  cmd_options.addStringOption("plugin", "p", EMPTYSTRING);
  cmd_options.addStringOption("wekafile", "w", EMPTYSTRING);
  cmd_options.addStringOption("filefeature", "f", EMPTYSTRING);
  cmd_options.addStringOption("extractor", "e", EMPTYSTRING);
  cmd_options.addNaturalOption("memory", "m", 40);
  cmd_options.addNaturalOption("nwinsamples", "ws", 512);
  cmd_options.addRealOption("samplingRate", "sr", 22050.0);
  cmd_options.addNaturalOption("accSize", "as", 1298);
  cmd_options.addNaturalOption("nhopsamples", "hp", 512);
  cmd_options.addStringOption("classifier", "c", EMPTYSTRING);
  cmd_options.addBoolOption("tline", "t", false);
  cmd_options.addBoolOption("pluginmute", "pm", false);
}

void 
loadOptions()
{
  helpopt = cmd_options.getBoolOption("help");
  usageopt = cmd_options.getBoolOption("usage");
  start = cmd_options.getRealOption("start");
  length = cmd_options.getRealOption("length");
  normopt = cmd_options.getBoolOption("normalize");

  pluginName = cmd_options.getStringOption("plugin");
  filefeaturename   = cmd_options.getStringOption("filefeature");
  wekafname = cmd_options.getStringOption("wekafile");
  extractorName = cmd_options.getStringOption("extractor");
  classifierName = cmd_options.getStringOption("classifier");
  memSize = cmd_options.getNaturalOption("memory");
  winSize = cmd_options.getNaturalOption("nwinsamples");
  hopSize = cmd_options.getNaturalOption("nhopsamples");
  samplingRate_ = cmd_options.getRealOption("samplingRate");
  accSize_ = cmd_options.getNaturalOption("accSize");
  tline = cmd_options.getBoolOption("tline");
  pluginMute  = cmd_options.getBoolOption("pluginmute");
}

void bextract(vector<string> soundfiles, mrs_natural label, 
	      string pluginName, string classNames, 
	      string wekafname,  mrs_natural memSize, 
	      string extractorStr,
	      string classifierName)
{
  MarSystemManager mng;

  // Spectrum Shape descriptors
  MarSystem* spectrumFeatures = mng.create("Fanout", "spectrumFeatures");
  if (extractorStr == "STFT") 
    {
      spectrumFeatures->addMarSystem(mng.create("Centroid", "cntrd"));
      spectrumFeatures->addMarSystem(mng.create("Rolloff", "rlf"));      
      spectrumFeatures->addMarSystem(mng.create("Flux", "flux"));
    }  
  else if (extractorStr == "MFCC")
    {
      spectrumFeatures->addMarSystem(mng.create("MFCC", "mfcc"));
    }
  else if (extractorStr == "STFTMFCC")
    {
      spectrumFeatures->addMarSystem(mng.create("Centroid", "cntrd"));
      spectrumFeatures->addMarSystem(mng.create("Rolloff", "rlf"));      
      spectrumFeatures->addMarSystem(mng.create("Flux", "flux"));
    }
  else
    {
      cout << "New bextract does not support extractor " << extractorStr << endl;
      return;
    }

  // Means and standard deviations of the spectrum features 
  // over a 1-second window (40 analysis frames) 
  MarSystem* textureFeatures = mng.create("Series", "textureFeatures");
  textureFeatures->addMarSystem(spectrumFeatures);
  textureFeatures->addMarSystem(mng.create("Memory", "textureMemory"));

  MarSystem* textureStats = mng.create("Fanout", "textureStats");
  textureStats->addMarSystem(mng.create("Mean", "mean"));
  textureStats->addMarSystem(mng.create("StandardDeviation", "std"));
  textureFeatures->addMarSystem(textureStats);

  // The main feature calculation network 
  MarSystem* spectralShape = mng.create("Series", "spectralShape");
  spectralShape->addMarSystem(mng.create("SoundFileSource", "src"));
  //spectralShape->addMarSystem(mng.create("AudioSink", "dest"));
  spectralShape->addMarSystem(mng.create("Windowing", "hamming"));
  spectralShape->addMarSystem(mng.create("Spectrum", "spk"));
  spectralShape->addMarSystem(mng.create("PowerSpectrum", "pspk"));
  spectralShape->addMarSystem(textureFeatures);

  // Label each class
  spectralShape->addMarSystem(mng.create("Annotator", "anno"));
  spectralShape->addMarSystem(mng.create("WekaSink", "wsink"));
  spectralShape->addMarSystem(mng.create("GaussianClassifier", "gsc"));
  spectralShape->addMarSystem(mng.create("Confidence", "conf"));

  vector<string>::iterator si;
  mrs_natural classCount = 0;

  if (wekafname == EMPTYSTRING) 
    spectralShape->updctrl("WekaSink/wsink/mrs_string/filename", "weka.arff");
  else 
    spectralShape->updctrl("WekaSink/wsink/mrs_string/filename", wekafname);

  spectralShape->updctrl("GaussianClassifier/gsc/mrs_natural/nLabels", (mrs_natural)soundfiles.size());

  spectralShape->linkctrl("mrs_bool/notEmpty", 
			  "SoundFileSource/src/mrs_bool/notEmpty");
  spectralShape->linkctrl("mrs_string/filename", 
			  "SoundFileSource/src/mrs_string/filename");
  spectralShape->linkctrl("mrs_natural/pos", 
			  "SoundFileSource/src/mrs_natural/pos");

  spectralShape->updctrl("GaussianClassifier/gsc/mrs_string/mode","train");  
  spectralShape->updctrl("WekaSink/wsink/mrs_string/labelNames",classNames);
  spectralShape->updctrl("Confidence/conf/mrs_string/labelNames",classNames);

  for (si = soundfiles.begin(); si != soundfiles.end(); ++si)
    {
      cout << "Processing class " << classCount << " collection: " 
	   << *si << endl;
      spectralShape->updctrl("SoundFileSource/src/mrs_string/filename", *si);
      spectralShape->updctrl("Annotator/anno/mrs_natural/label", classCount);
      classCount ++;

      while(spectralShape->getctrl("mrs_bool/notEmpty")->toBool()) 
	{
	  spectralShape->tick();
	}
    }
  spectralShape->updctrl("GaussianClassifier/gsc/mrs_bool/done",true);

  spectralShape->tick(); // train classifier 
	
  spectralShape->updctrl("GaussianClassifier/gsc/mrs_string/mode","predict");     

  if (pluginName == EMPTYSTRING) // output to stdout 
    cout << (*spectralShape) << endl;      
  else 
    {
      ofstream oss(pluginName.c_str());
      oss << (*spectralShape) << endl;
    }
}

int
main(int argc, const char **argv)
{
  MRSDIAG("sfplay.cpp - main");

  //////////////////////////////////////////////////////////////////////////
  // Supported Feature extractors
  //////////////////////////////////////////////////////////////////////////
  // list of supported feature extractors and their creation functions
  featExtractors["MPL_FILE"] = &createExtractorFromFile;
  featExtractors["STFT"] = &createSTFTextractor;
  featExtractors["MFCC"] = &createMFCCextractor;
  featExtractors["STFTMFCC"] = &createSTFTMFCCextractor;
  featExtractors["SCF"] = &createSCFextractor;
  featExtractors["SFM"] = &createSFMextractor;
  featExtractors["SFMSCF"] = &createSFMSCFextractor;
  featExtractors["LSP"] = &createLSPextractor;
  featExtractors["LPCC"] = &createLPCCextractor;
  featExtractors["BEAT"] = &createBEATextrator;
  // short description of each extractor
  featExtractorDesc["MPL_FILE"] = "not yet implemented...";
  featExtractorDesc["STFT"] = "Centroid, Rolloff, Flux, ZeroCrossings ";
  featExtractorDesc["MFCC"] = "Mel-frequency Cepstral Coefficients ";
  featExtractorDesc["STFTMFCC"] = "Centroid, Rolloff Flux, ZeroCrossings, Mel-frequency Cepstral Coefficients";
  featExtractorDesc["SCF"] = "Spectral Crest Factor (MPEG-7)";
  featExtractorDesc["SFM"] = "Spectral Flatness Measure (MPEG-7)";
  featExtractorDesc["SFMSCF"] = "SCF and SFM features";
  featExtractorDesc["LSP"] = "Linear Spectral Pairs";
  featExtractorDesc["LPCC"] = "LPC derived Cepstral coefficients ";
  featExtractorDesc["BEAT"] = "Beat histogram features";
  //////////////////////////////////////////////////////////////////////////

  string progName = argv[0];  
  if (argc == 1)
    {
      printUsage(progName);
      return 0;
    }
  

  initOptions();
  cmd_options.readOptions(argc, argv);
  loadOptions();



  if (helpopt) 
    printHelp(progName);

  if (usageopt)
    printUsage(progName);


  //////////////////////////////////////////////////////////////////////////
  // Print analysis options
  //////////////////////////////////////////////////////////////////////////
  cout << endl;
  cout << "Window Size (in samples): " << winSize << endl;
  cout << "Hop Size (in samples): " << hopSize << endl;
  cout << "Memory Size (in analysis windows):" << memSize << endl;
  cout << "Sampling rate: " << samplingRate_ << endl;
  cout << "Accumulator size (in analysis windows):" << accSize_ << endl;
  cout << endl;
  cout << "Extractor = " << extractorName << endl;
  cout << endl;
  



  int i = 0;
  Collection l;

  string classNames = "";
  vector<Collection> cls;

  vector<string> soundfiles = cmd_options.getRemaining();
  vector<string>::iterator sfi;

  for (sfi = soundfiles.begin(); sfi != soundfiles.end(); ++sfi) 
    {	
      string sfname = *sfi;
      Collection l;
      readCollection(l,sfname);

      classNames += (l.name()+',');
      cls.push_back(l);
      i++;
    }

  string extractorStr = extractorName;

  //SINGLE-VALUE Extractor
  if (extractorStr.substr(0,2) == "SV") 
    {
      bool withBeatFeatures = false;
      string extrName;

      //BEAT Extractor
      if (extractorName.substr(extractorName.length()-4, extractorName.length()) == "BEAT")
	{
	  withBeatFeatures = true;
	  extrName = extractorName.substr(2, extractorName.length()-6);
	}
      else 
	extrName = extractorName.substr(2, extractorName.length());


      cout << "extrName = " << extrName << endl;
      

      if(featExtractors.find(extrName)== featExtractors.end())
	{
	  cout << extractorStr << ": Unsupported extractor!" << endl;
	  return 1;
	}

      bextract_trainAccumulator(cls, i, pluginName, classNames, wekafname, filefeaturename, memSize, extrName,
				withBeatFeatures);
    }
  //REMOVE_SILENCE Extractor
  else if (extractorStr.substr(0,2) == "RS")
    {
      string extrName;
      extrName = extractorName.substr(2, extractorName.length());	
      if(featExtractors.find(extrName)== featExtractors.end())
	{
	  cout << extractorStr << ": Unsupported extractor!" << endl;
	  return 1;
	}

      bextract_train_rmsilence(cls, i, pluginName, classNames, wekafname, memSize, extrName, classifierName);      
    }
  else if (extractorStr == "STEREOSPS")
    {
      bextract_trainStereoSPS(cls, classNames, wekafname, memSize);      
    }
  else if (extractorStr == "STEREOMFCC")
    {
      bextract_trainStereoMFCC(cls, classNames, wekafname, memSize);            
    }
  else if (extractorStr == "STEREOSPSMFCC")
    {
      bextract_trainStereoSPSMFCC(cls, classNames, wekafname, memSize);            
    }

  //NORMAL Extractor
  else
    {
      if(featExtractors.find(extractorStr)== featExtractors.end())
	{
	  cout << extractorStr << ": Unsupported extractor!" << endl;
	  return 1;
	}
		
      bextract_train(cls, i, pluginName, classNames, wekafname, memSize, 
		     extractorName, classifierName);
    }

  return 0;
}








