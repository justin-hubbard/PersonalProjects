/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
KadenzeChorusFlangerAudioProcessor::KadenzeChorusFlangerAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      .withOutput ("Output", AudioChannelSet::stereo(), true))
{
    
    // Construct and add parameters
    addParameter(mDryWetParameter = new AudioParameterFloat("drywet", "Dry/Wet", 0.0, 1.0, 0.5));
    addParameter(mDepthParameter = new AudioParameterFloat("depth", "Depth", 0.0, 1.0, 0.5));
    addParameter(mRateParameter = new AudioParameterFloat("rate", "Rate", 0.1f, 20.0f, 10.0f));
    addParameter(mPhaseOffsetParameter = new AudioParameterFloat("phaseoffset", "Phase Offset", 0.0f, 1.0f, 0.f));
    addParameter(mFeedbackParameter = new AudioParameterFloat("feedback", "Feedback", 0 , 0.98, 0.5));
    addParameter(mTypeParameter = new AudioParameterFloat("type", "Type", 0, 1, 0));
    
    // Initialize data to default values
    mCircularBufferLeft = nullptr;
    mCircularBufferRight = nullptr;
    mCircularBufferWriteHead = 0;
    mCircularBufferLength = 0;
    
    mFeedbackLeft = 0;
    mFeedbackRight = 0;
    
    mLFOPhase = 0;
}

KadenzeChorusFlangerAudioProcessor::~KadenzeChorusFlangerAudioProcessor()
{
    if (mCircularBufferLeft != nullptr)
    {
        delete [] mCircularBufferLeft;
        mCircularBufferLeft = nullptr;
    }
    if (mCircularBufferRight != nullptr)
    {
        delete [] mCircularBufferRight;
        mCircularBufferRight = nullptr;
    }
}

//==============================================================================
const String KadenzeChorusFlangerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool KadenzeChorusFlangerAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool KadenzeChorusFlangerAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool KadenzeChorusFlangerAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double KadenzeChorusFlangerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int KadenzeChorusFlangerAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int KadenzeChorusFlangerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void KadenzeChorusFlangerAudioProcessor::setCurrentProgram (int index)
{
}

const String KadenzeChorusFlangerAudioProcessor::getProgramName (int index)
{
    return {};
}

void KadenzeChorusFlangerAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void KadenzeChorusFlangerAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Initialize data for the current sample rate, reset phase, writeheads, etc
    
    // Initialize the phase
    mLFOPhase = 0;
    
    // Calculate circular buffer length
    mCircularBufferLength = sampleRate * MAX_DELAY_TIME;
    
    
    // Initialize left buffer
    if (mCircularBufferLeft == nullptr)
    {
        mCircularBufferLeft = new float[mCircularBufferLength];
    }
    // Clear any junk data in the buffer
    zeromem(mCircularBufferLeft, mCircularBufferLength * sizeof(float));
    
    // Initialize right buffer
    if (mCircularBufferRight == nullptr)
    {
        mCircularBufferRight = new float[mCircularBufferLength];
    }
    // Clear any junk data in the buffer
    zeromem(mCircularBufferRight, mCircularBufferLength * sizeof(float));
    
    // Init write head to 0
    mCircularBufferWriteHead = 0;
}

void KadenzeChorusFlangerAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool KadenzeChorusFlangerAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() == AudioChannelSet::stereo() &&
        layouts.getMainOutputChannelSet() == AudioChannelSet::stereo())
    {
        return true;
    }
    else
    {
        return false;
    }
}

void KadenzeChorusFlangerAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    
    // Obtain left and right audio data pointers
    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);
    
    // Iterate through all samples in the buffer
    for (int i = 0; i < buffer.getNumSamples(); i++)
    {
        // Write into our circular buffer
        mCircularBufferLeft[mCircularBufferWriteHead] = leftChannel[i] + mFeedbackLeft;
        mCircularBufferRight[mCircularBufferWriteHead] = rightChannel[i] + mFeedbackRight;
        
        
        //LFO start
        // Generate left LFO output
        float lfoOutLeft = sin(2*M_PI * mLFOPhase);
        
        // Calculate right channel LFO phase
        float lfoPhaseRight = mLFOPhase + *mPhaseOffsetParameter;
        
        if (lfoPhaseRight > 1) {
            lfoPhaseRight -= 1;
        }
        // Generate right LFO output
        float lfoOutRight = sin(2*M_PI * lfoPhaseRight);
        
        // Move our LFO phase forward
        mLFOPhase += *mRateParameter / getSampleRate();
        
        if (mLFOPhase > 1)
            mLFOPhase -= 1;

        // Control the LFO depth by mult output by depth param
        lfoOutLeft *= *mDepthParameter;
        lfoOutRight *= *mDepthParameter;
        
        float lfoOutMappedLeft;
        float lfoOutMappedRight;
        
        // Map LFO outputs to desired delay times (chorus/flanger)
        //
        // Chorus
        if (*mTypeParameter == 0)
        {
            lfoOutMappedLeft = jmap(lfoOutLeft, -1.f, 1.f, 0.005f, 0.03f);
            lfoOutMappedRight = jmap(lfoOutRight, -1.f, 1.f, 0.005f, 0.03f);
        }
        else
        {
            lfoOutMappedLeft = jmap(lfoOutLeft, -1.f, 1.f, 0.001f, 0.005f);
            lfoOutMappedRight = jmap(lfoOutRight, -1.f, 1.f, 0.001f, 0.005f);
        }
        
        // Calculate delay lengths in samples
        float delayTimeSamplesLeft = getSampleRate() * lfoOutMappedLeft;
        float delayTimeSamplesRight = getSampleRate() * lfoOutMappedRight;
        // LFO end
        
        // Calculate left read head position
        float delayReadHeadLeft = mCircularBufferWriteHead - delayTimeSamplesLeft;
        
        if (delayReadHeadLeft < 0)
        {
            delayReadHeadLeft += mCircularBufferLength;
        }
        
        // Calculate right read head position
        float delayReadHeadRight = mCircularBufferWriteHead - delayTimeSamplesRight;
        if (delayReadHeadRight < 0)
        {
            delayReadHeadRight += mCircularBufferLength;
        }
            
        // interpolation
        // Calculate linear interp points for left channel
        int readHeadLeft_x = (int)delayReadHeadLeft;
        int readHeadLeft_x1 = readHeadLeft_x + 1;
        float readHeadFloatLeft = delayReadHeadLeft - readHeadLeft_x;
        
        if (readHeadLeft_x1 >= mCircularBufferLength)
        {
            readHeadLeft_x1 -= mCircularBufferLength;
        }
        
        // Calculate linear interp points for right channel
        int readHeadRight_x = (int)delayReadHeadRight;
        int readHeadRight_x1 = readHeadRight_x + 1;
        float readHeadFloatRight = delayReadHeadRight - readHeadRight_x;
        
        if (readHeadRight_x1 >= mCircularBufferLength)
        {
            readHeadRight_x1 -= mCircularBufferLength;
        }
        // interp
        
        // Generate left and right output channels
        float delay_sample_left = lin_interp(mCircularBufferLeft[readHeadLeft_x], mCircularBufferLeft[readHeadLeft_x1], readHeadFloatLeft);
        float delay_sample_right = lin_interp(mCircularBufferRight[readHeadRight_x], mCircularBufferRight[readHeadRight_x1], readHeadFloatRight);
        
        mFeedbackLeft = delay_sample_left * *mFeedbackParameter;
        mFeedbackRight = delay_sample_right * *mFeedbackParameter;
        
        mCircularBufferWriteHead++;
        if (mCircularBufferWriteHead >= mCircularBufferLength)
        {
            mCircularBufferWriteHead = 0;
        }
        
        float dryAmount = 1 - *mDryWetParameter;
        float wetAmount = *mDryWetParameter;
        
        buffer.setSample(0, i, buffer.getSample(0, i) * dryAmount + delay_sample_left * wetAmount);
        buffer.setSample(1, i, buffer.getSample(1, i) * dryAmount + delay_sample_right *  wetAmount);
    }
}

//==============================================================================
bool KadenzeChorusFlangerAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* KadenzeChorusFlangerAudioProcessor::createEditor()
{
    return new KadenzeChorusFlangerAudioProcessorEditor (*this);
}

//==============================================================================
void KadenzeChorusFlangerAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    std::unique_ptr<XmlElement> xml(new XmlElement("FlangerChorus"));
    
    xml->setAttribute("DryWet", *mDryWetParameter);
    xml->setAttribute("Depth", *mDepthParameter);
    xml->setAttribute("Rate", *mRateParameter);
    xml->setAttribute("PhaseOffset", *mPhaseOffsetParameter);
    xml->setAttribute("Feedback", *mFeedbackParameter);
    xml->setAttribute("Type", *mTypeParameter);
    
    copyXmlToBinary(*xml, destData);
    
}

void KadenzeChorusFlangerAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    std::unique_ptr<XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    
    if (xml.get() != nullptr && xml->hasTagName("FlangerChorus"))
    {
        *mDryWetParameter = xml->getDoubleAttribute("DryWet");
        *mDepthParameter = xml->getDoubleAttribute("Depth");
        *mRateParameter = xml->getDoubleAttribute("Rate");
        *mPhaseOffsetParameter = xml->getDoubleAttribute("PhaseOffset");
        *mFeedbackParameter = xml->getDoubleAttribute("Feedback");
        *mTypeParameter = xml->getIntAttribute("Type");
    }
    
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new KadenzeChorusFlangerAudioProcessor();
}

float KadenzeChorusFlangerAudioProcessor::lin_interp(float inSampleX, float inSampleY, float inFloatPhase)
{
    return (1 - inFloatPhase) * inSampleX + inFloatPhase * inSampleY;
}
