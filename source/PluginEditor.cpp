#include "PluginEditor.h"

namespace
{
    using namespace juce;
    Colour bg()        { return Colour(0xff050816); }
    Colour bg2()       { return Colour(0xff120826); }
    Colour panel()     { return Colour(0xff0b1220); }
    Colour panel2()    { return Colour(0xff131d31); }
    Colour edge()      { return Colour(0x6656d8ff); }
    Colour accent()    { return Colour(0xff69f0ff); }
    Colour accent2()   { return Colour(0xffff5cf4); }
    Colour accent3()   { return Colour(0xff7fff87); }
    Colour amber()     { return Colour(0xffffb45c); }
    Colour textHi()    { return Colours::white.withAlpha(0.96f); }
    Colour textLo()    { return Colours::white.withAlpha(0.66f); }

    void drawCard(Graphics& g, Rectangle<float> r, const String& title, Colour c1 = panel2(), Colour c2 = panel())
    {
        g.setGradientFill(ColourGradient(c1, r.getTopLeft(), c2, r.getBottomRight(), false));
        g.fillRoundedRectangle(r, 14.0f);
        g.setColour(edge());
        g.drawRoundedRectangle(r, 14.0f, 1.2f);
        g.setColour(Colours::white.withAlpha(0.04f));
        g.fillRoundedRectangle(r.removeFromTop(34.0f), 14.0f);
        g.setColour(textHi());
        g.setFont(FontOptions(13.0f, Font::bold));
        auto titleArea = juce::Rectangle<int>(
            juce::roundToInt(r.getX() + 12.0f),
            juce::roundToInt(r.getY() + 7.0f),
            juce::roundToInt(r.getWidth() - 24.0f),
            20);
        g.drawText(title, titleArea, Justification::left);
    }

    StringArray octaveItems() { return { "-4","-3","-2","-1","0","1","2","3","4" }; }
    StringArray semiItems()   { return { "-12","-11","-10","-9","-8","-7","-6","-5","-4","-3","-2","-1","0","1","2","3","4","5","6","7","8","9","10","11","12" }; }
    StringArray unisonItems() { return { "1","2","3","4","5" }; }
    StringArray subWaveItems(){ return { "Sine","Tri","Saw","Square" }; }
    StringArray filterItems() { return { "LP", "BP", "HP" }; }
    StringArray playItems()   { return { "Mono", "Legato", "Poly" }; }
    StringArray polyItems()   { return { "1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16" }; }
    StringArray modSources()  { return { "None", "ENV2", "ENV3", "LFO1", "LFO2", "LFO3", "LFO4", "Velocity" }; }
    StringArray modDests()    { return { "None", "Cutoff", "OSC A Pos", "OSC B Pos", "Pitch", "Level", "Pan", "Dist", "FX" }; }
}

void ScopeDisplay::timerCallback() { processor.copyScopeData(data); repaint(); }
void SpectrumDisplay::timerCallback() { processor.copyAnalyzerData(data); repaint(); }
void LfoDisplay::timerCallback() { processor.copyLfoShape(data); repaint(); }
void MatrixDisplay::timerCallback() { summary = processor.getMatrixSummary(); repaint(); }

void ScopeDisplay::paint(Graphics& g)
{
    g.fillAll(Colours::transparentBlack);
    auto r = getLocalBounds().toFloat().reduced(6.0f);
    g.setColour(Colours::black.withAlpha(0.25f));
    g.fillRoundedRectangle(r, 8.0f);
    g.setColour(accent());
    Path p;
    p.startNewSubPath(r.getX(), r.getCentreY());
    for (size_t i = 0; i < data.size(); ++i)
    {
        const float x = r.getX() + r.getWidth() * (float) i / (float) (data.size() - 1);
        const float y = r.getCentreY() - data[i] * r.getHeight() * 0.42f;
        p.lineTo(x, y);
    }
    g.strokePath(p, PathStrokeType(2.0f));
}

void SpectrumDisplay::paint(Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced(6.0f);
    g.setColour(Colours::black.withAlpha(0.25f));
    g.fillRoundedRectangle(r, 8.0f);
    g.setColour(accent2());
    for (size_t i = 0; i < data.size(); ++i)
    {
        const float x = r.getX() + r.getWidth() * (float) i / (float) data.size();
        const float w = r.getWidth() / (float) data.size();
        const float norm = jlimit(0.0f, 1.0f, (data[i] + 100.0f) / 100.0f);
        g.fillRoundedRectangle(x, r.getBottom() - norm * r.getHeight(), w - 1.0f, norm * r.getHeight(), 1.0f);
    }
}

void LfoDisplay::paint(Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced(6.0f);
    g.setColour(Colours::black.withAlpha(0.25f));
    g.fillRoundedRectangle(r, 8.0f);
    g.setColour(Colours::white.withAlpha(0.18f));
    g.drawLine(r.getX(), r.getCentreY(), r.getRight(), r.getCentreY(), 1.0f);
    g.setColour(Colours::yellow.withAlpha(0.95f));
    Path p;
    p.startNewSubPath(r.getX(), r.getCentreY());
    for (size_t i = 0; i < data.size(); ++i)
    {
        const float x = r.getX() + r.getWidth() * (float) i / (float) (data.size() - 1);
        const float y = r.getCentreY() - data[i] * r.getHeight() * 0.40f;
        p.lineTo(x, y);
    }
    g.strokePath(p, PathStrokeType(2.0f));
}

void MatrixDisplay::paint(Graphics& g)
{
    auto r = getLocalBounds().reduced(10).toFloat();
    g.setColour(Colours::black.withAlpha(0.2f));
    g.fillRoundedRectangle(r, 8.0f);
    g.setColour(textLo());
    g.setFont(FontOptions(12.0f));
    g.drawFittedText(summary, r.toNearestInt(), Justification::topLeft, 12);
}

SerumKnob::SerumKnob()
{
    addAndMakeVisible(slider);
    addAndMakeVisible(title);
    slider.setSliderStyle(Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(Slider::TextBoxBelow, false, 56, 16);
    slider.setColour(Slider::rotarySliderFillColourId, accent());
    slider.setColour(Slider::thumbColourId, accent2());
    slider.setColour(Slider::textBoxTextColourId, textHi());
    slider.setColour(Slider::textBoxOutlineColourId, Colours::transparentBlack);
    slider.setColour(Slider::textBoxBackgroundColourId, Colours::black.withAlpha(0.22f));
    title.setJustificationType(Justification::centred);
    title.setColour(Label::textColourId, textLo());
    title.setFont(FontOptions(11.0f));
}

void SerumKnob::resized()
{
    auto r = getLocalBounds();
    title.setBounds(r.removeFromTop(18));
    slider.setBounds(r);
}

FiveParksVST3AudioProcessorEditor::FiveParksVST3AudioProcessorEditor(FiveParksVST3AudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), scope(p), spectrum(p), lfoShape(p), matrixView(p)
{
    setSize(1600, 940);

    logo.setText("5parks VST3", dontSendNotification);
    logo.setColour(juce::Label::textColourId, textHi());
    logo.setFont(juce::FontOptions(28.0f, juce::Font::bold));
    subtitle.setText("Full sound suite · colourful future UI · layered bass design · modulation · FX", dontSendNotification);
    subtitle.setColour(juce::Label::textColourId, textLo());
    subtitle.setFont(juce::FontOptions(13.0f));
    addAndMakeVisible(logo);
    addAndMakeVisible(subtitle);

    for (auto* c : { &oscABox, &oscBBox, &subBox, &noiseBox, &filterBox, &bassBox, &charBox, &envBox, &lfoBox, &matrixBox, &fxBox, &spaceBox, &perfBox, &displayBox })
        addAndMakeVisible(*c);

    displayBox.addAndMakeVisible(scope);
    displayBox.addAndMakeVisible(spectrum);
    displayBox.addAndMakeVisible(lfoShape);
    matrixBox.addAndMakeVisible(matrixView);

    oscABox.addAndMakeVisible(oscAOct); oscABox.addAndMakeVisible(oscASemi); oscABox.addAndMakeVisible(oscAUnison);
    oscBBox.addAndMakeVisible(oscBOct); oscBBox.addAndMakeVisible(oscBSemi); oscBBox.addAndMakeVisible(oscBUnison);
    for (auto* cb : { &oscAOct,&oscASemi,&oscAUnison,&oscBOct,&oscBSemi,&oscBUnison,&subWave,&filterMode,&playMode,&polyphony,&mod1Src,&mod1Dst,&mod2Src,&mod2Dst,&mod3Src,&mod3Dst,&mod4Src,&mod4Dst,&mod5Src,&mod5Dst,&mod6Src,&mod6Dst })
        styleCombo(*cb);

    oscAOct.addItemList(octaveItems(), 1); oscASemi.addItemList(semiItems(), 1); oscAUnison.addItemList(unisonItems(), 1);
    oscBOct.addItemList(octaveItems(), 1); oscBSemi.addItemList(semiItems(), 1); oscBUnison.addItemList(unisonItems(), 1);
    subWave.addItemList(subWaveItems(), 1); filterMode.addItemList(filterItems(), 1); playMode.addItemList(playItems(), 1); polyphony.addItemList(polyItems(), 1);
    for (auto* cb : { &mod1Src,&mod2Src,&mod3Src,&mod4Src,&mod5Src,&mod6Src }) cb->addItemList(modSources(), 1);
    for (auto* cb : { &mod1Dst,&mod2Dst,&mod3Dst,&mod4Dst,&mod5Dst,&mod6Dst }) cb->addItemList(modDests(), 1);

    subDirect.setButtonText("Direct Out");
    noiseDirect.setButtonText("Direct Out");
    for (auto* b : { &subDirect, &noiseDirect })
    {
        b->setColour(ToggleButton::textColourId, textLo());
        b->setColour(ToggleButton::tickColourId, accent());
        b->setColour(ToggleButton::tickDisabledColourId, textLo());
    }
    subBox.addAndMakeVisible(subWave); subBox.addAndMakeVisible(subDirect);
    noiseBox.addAndMakeVisible(noiseDirect);
    filterBox.addAndMakeVisible(filterMode);
    perfBox.addAndMakeVisible(playMode); perfBox.addAndMakeVisible(polyphony);

    attach(oscABox, oscAPos, "oscA_pos", "WT POS");
    attach(oscABox, oscALevel, "oscA_level", "LEVEL");
    attach(oscABox, oscADetune, "oscA_detune", "DETUNE");
    attach(oscABox, oscABlend, "oscA_blend", "BLEND");
    attach(oscABox, oscAPan, "oscA_pan", "PAN");

    attach(oscBBox, oscBPos, "oscB_pos", "WT POS");
    attach(oscBBox, oscBLevel, "oscB_level", "LEVEL");
    attach(oscBBox, oscBDetune, "oscB_detune", "DETUNE");
    attach(oscBBox, oscBBlend, "oscB_blend", "BLEND");
    attach(oscBBox, oscBPan, "oscB_pan", "PAN");

    attach(subBox, subLevel, "sub_level", "LEVEL");
    attach(noiseBox, noiseLevel, "noise_level", "LEVEL");
    attach(filterBox, filterCutoff, "filter_cutoff", "CUTOFF");
    attach(filterBox, filterRes, "filter_res", "RES");
    attach(filterBox, filterDrive, "filter_drive", "DRIVE");
    attach(filterBox, filterMix, "filter_mix", "MIX");

    attach(bassBox, reeseAmt, "reese_amt", "REESE");
    attach(bassBox, warhornAmt, "warhorn_amt", "WARHORN");
    attach(bassBox, wompAmt, "womp_amt", "WOMP");
    attach(bassBox, subharmAmt, "subharm_amt", "SUB HARM");
    attach(bassBox, comboAmt, "combo_amt", "COMBO");
    attach(bassBox, stereoWidth, "stereo_width", "WIDTH");
    attach(bassBox, fmAmt, "fm_amt", "FM GRIT");
    attach(bassBox, airAmt, "air_amt", "AIR");

    attach(charBox, bodyAmt, "body_amt", "BODY");
    attach(charBox, growlAmt, "growl_amt", "GROWL");
    attach(charBox, shimmerAmt, "shimmer_amt", "SHIMMER");
    attach(charBox, motionAmt, "motion_amt", "MOTION");

    attach(envBox, env1A, "env1_a", "ATTACK");
    attach(envBox, env1D, "env1_d", "DECAY");
    attach(envBox, env1S, "env1_s", "SUSTAIN");
    attach(envBox, env1R, "env1_r", "RELEASE");

    attach(lfoBox, lfo1Rate, "lfo1_rate", "RATE");
    attach(lfoBox, lfo1Amt, "lfo1_amt", "AMOUNT");
    attach(lfoBox, lfo1Shape, "lfo1_shape", "SHAPE");
    attach(lfoBox, lfo1Smooth, "lfo1_smooth", "SMOOTH");

    attach(fxBox, distDrive, "dist_drive", "DIST DRIVE");
    attach(fxBox, distMix, "dist_mix", "DIST MIX");
    attach(fxBox, chorusMix, "chorus_mix", "CHORUS");
    attach(fxBox, delayMix, "delay_mix", "DELAY");
    attach(fxBox, reverbMix, "reverb_mix", "REVERB");
    attach(fxBox, compAmt, "comp_amt", "COMP");
    attach(spaceBox, reverbSize, "reverb_size", "SIZE");
    attach(spaceBox, reverbTone, "reverb_tone", "TONE");
    attach(spaceBox, chorusRate, "chorus_rate", "CH RATE");
    attach(spaceBox, delayTime, "delay_time", "DL TIME");

    attach(perfBox, glide, "glide_ms", "GLIDE");
    attach(perfBox, masterGain, "master_gain", "MASTER");

    addMatrixSlot(1, mod1Src, mod1Dst, mod1Amt, "mod1_src", "mod1_dst", "mod1_amt");
    addMatrixSlot(2, mod2Src, mod2Dst, mod2Amt, "mod2_src", "mod2_dst", "mod2_amt");
    addMatrixSlot(3, mod3Src, mod3Dst, mod3Amt, "mod3_src", "mod3_dst", "mod3_amt");
    addMatrixSlot(4, mod4Src, mod4Dst, mod4Amt, "mod4_src", "mod4_dst", "mod4_amt");
    addMatrixSlot(5, mod5Src, mod5Dst, mod5Amt, "mod5_src", "mod5_dst", "mod5_amt");
    addMatrixSlot(6, mod6Src, mod6Dst, mod6Amt, "mod6_src", "mod6_dst", "mod6_amt");

    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, "oscA_oct", oscAOct));
    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, "oscA_semi", oscASemi));
    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, "oscA_unison", oscAUnison));
    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, "oscB_oct", oscBOct));
    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, "oscB_semi", oscBSemi));
    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, "oscB_unison", oscBUnison));
    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, "sub_wave", subWave));
    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, "filter_mode", filterMode));
    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, "play_mode", playMode));
    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, "polyphony", polyphony));
    buttonAtts.push_back(std::make_unique<ButtonAttachment>(audioProcessor.apvts, "sub_direct", subDirect));
    buttonAtts.push_back(std::make_unique<ButtonAttachment>(audioProcessor.apvts, "noise_direct", noiseDirect));
}

void FiveParksVST3AudioProcessorEditor::configureKnob(SerumKnob& knob, const String& title)
{
    knob.title.setText(title, dontSendNotification);
}

void FiveParksVST3AudioProcessorEditor::attach(Component& parent, SerumKnob& knob, const String& id, const String& title)
{
    configureKnob(knob, title);
    parent.addAndMakeVisible(knob);
    sliderAtts.push_back(std::make_unique<SliderAttachment>(audioProcessor.apvts, id, knob.slider));
}

void FiveParksVST3AudioProcessorEditor::styleCombo(ComboBox& box)
{
    box.setColour(ComboBox::backgroundColourId, Colours::black.withAlpha(0.22f));
    box.setColour(ComboBox::outlineColourId, edge());
    box.setColour(ComboBox::textColourId, textHi());
    box.setColour(ComboBox::arrowColourId, accent());
}

void FiveParksVST3AudioProcessorEditor::addMatrixSlot(int, ComboBox& src, ComboBox& dst, SerumKnob& amt, const String& srcId, const String& dstId, const String& amtId)
{
    matrixBox.addAndMakeVisible(src);
    matrixBox.addAndMakeVisible(dst);
    attach(matrixBox, amt, amtId, "AMT");
    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, srcId, src));
    comboAtts.push_back(std::make_unique<ComboAttachment>(audioProcessor.apvts, dstId, dst));
}

void FiveParksVST3AudioProcessorEditor::paint(Graphics& g)
{
    auto area = getLocalBounds().toFloat();
    g.setGradientFill(ColourGradient(bg2(), area.getTopLeft(), bg(), area.getBottomRight(), false));
    g.fillAll(bg());
    g.setGradientFill(ColourGradient(bg2(), area.getTopLeft(), bg(), area.getBottomRight(), false));
    g.fillRect(area);

    for (int y = 0; y < getHeight(); y += 28)
    {
        g.setColour(Colours::white.withAlpha((y % 56 == 0) ? 0.05f : 0.025f));
        g.drawHorizontalLine(y, 0.0f, (float) getWidth());
    }
    for (int x = 0; x < getWidth(); x += 28)
    {
        g.setColour(Colours::white.withAlpha((x % 56 == 0) ? 0.05f : 0.025f));
        g.drawVerticalLine(x, 0.0f, (float) getHeight());
    }

    g.setColour(accent().withAlpha(0.06f));
    g.fillEllipse(30.0f, 20.0f, 360.0f, 220.0f);
    g.setColour(accent2().withAlpha(0.05f));
    g.fillEllipse((float) getWidth() - 420.0f, 30.0f, 360.0f, 260.0f);

    drawCard(g, displayBox.getBounds().toFloat(), "VISUALS", Colour(0xff10253a), Colour(0xff08121d));
    drawCard(g, oscABox.getBounds().toFloat(), "OSC A", Colour(0xff1a1c35), Colour(0xff111425));
    drawCard(g, oscBBox.getBounds().toFloat(), "OSC B", Colour(0xff16233f), Colour(0xff101629));
    drawCard(g, subBox.getBounds().toFloat(), "SUB", Colour(0xff183221), Colour(0xff111d18));
    drawCard(g, noiseBox.getBounds().toFloat(), "NOISE", Colour(0xff352218), Colour(0xff211512));
    drawCard(g, filterBox.getBounds().toFloat(), "FILTER", Colour(0xff1e203b), Colour(0xff161829));
    drawCard(g, bassBox.getBounds().toFloat(), "BASS ENGINE", Colour(0xff10263f), Colour(0xff0d1624));
    drawCard(g, charBox.getBounds().toFloat(), "CHARACTER", Colour(0xff301738), Colour(0xff1b1123));
    drawCard(g, envBox.getBounds().toFloat(), "ENV 1", Colour(0xff162a33), Colour(0xff111b24));
    drawCard(g, lfoBox.getBounds().toFloat(), "LFO 1", Colour(0xff1d2436), Colour(0xff131926));
    drawCard(g, matrixBox.getBounds().toFloat(), "MOD MATRIX", Colour(0xff11263c), Colour(0xff0d1622));
    drawCard(g, fxBox.getBounds().toFloat(), "FX", Colour(0xff32231b), Colour(0xff1d1512));
    drawCard(g, spaceBox.getBounds().toFloat(), "SPACE", Colour(0xff142f34), Colour(0xff101d20));
    drawCard(g, perfBox.getBounds().toFloat(), "PERFORMANCE", Colour(0xff252b1a), Colour(0xff181d12));
}

void FiveParksVST3AudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(14);
    auto header = area.removeFromTop(56);
    logo.setBounds(header.removeFromLeft(320));
    subtitle.setBounds(header.removeFromLeft(760));
    area.removeFromTop(8);

    auto top = area.removeFromTop(248);
    displayBox.setBounds(top.removeFromLeft(420));
    top.removeFromLeft(8);
    oscABox.setBounds(top.removeFromLeft(260));
    top.removeFromLeft(8);
    oscBBox.setBounds(top.removeFromLeft(260));
    top.removeFromLeft(8);
    auto sideTop = top;
    subBox.setBounds(sideTop.removeFromTop(118));
    sideTop.removeFromTop(8);
    noiseBox.setBounds(sideTop);

    area.removeFromTop(10);
    auto middle = area.removeFromTop(254);
    filterBox.setBounds(middle.removeFromLeft(248));
    middle.removeFromLeft(8);
    bassBox.setBounds(middle.removeFromLeft(360));
    middle.removeFromLeft(8);
    charBox.setBounds(middle.removeFromLeft(248));
    middle.removeFromLeft(8);
    envBox.setBounds(middle.removeFromLeft(248));
    middle.removeFromLeft(8);
    lfoBox.setBounds(middle);

    area.removeFromTop(10);
    auto bottom = area;
    matrixBox.setBounds(bottom.removeFromLeft(700));
    bottom.removeFromLeft(8);
    fxBox.setBounds(bottom.removeFromLeft(300));
    bottom.removeFromLeft(8);
    spaceBox.setBounds(bottom.removeFromLeft(210));
    bottom.removeFromLeft(8);
    perfBox.setBounds(bottom);

    auto disp = displayBox.getLocalBounds().reduced(12); disp.removeFromTop(24);
    scope.setBounds(disp.removeFromTop(84));
    disp.removeFromTop(6);
    spectrum.setBounds(disp.removeFromTop(84));
    disp.removeFromTop(6);
    lfoShape.setBounds(disp.removeFromTop(84));

    auto layoutOsc = [](Component& box, ComboBox& oct, ComboBox& semi, ComboBox& uni, std::initializer_list<SerumKnob*> knobs)
    {
        auto r = box.getLocalBounds().reduced(10); r.removeFromTop(24);
        auto head = r.removeFromTop(24);
        oct.setBounds(head.removeFromLeft(70)); head.removeFromLeft(6); semi.setBounds(head.removeFromLeft(72)); head.removeFromLeft(6); uni.setBounds(head.removeFromLeft(70));
        r.removeFromTop(8);
        auto row1 = r.removeFromTop(148);
        for (auto* k : knobs)
        {
            k->setBounds(row1.removeFromLeft(48).reduced(2));
            row1.removeFromLeft(2);
        }
    };
    layoutOsc(oscABox, oscAOct, oscASemi, oscAUnison, { &oscAPos, &oscALevel, &oscADetune, &oscABlend, &oscAPan });
    layoutOsc(oscBBox, oscBOct, oscBSemi, oscBUnison, { &oscBPos, &oscBLevel, &oscBDetune, &oscBBlend, &oscBPan });

    auto sb = subBox.getLocalBounds().reduced(10); sb.removeFromTop(24);
    auto srow = sb.removeFromTop(24);
    subWave.setBounds(srow.removeFromLeft(90)); srow.removeFromLeft(8); subDirect.setBounds(srow.removeFromLeft(110));
    subLevel.setBounds(sb.removeFromTop(138).removeFromLeft(66));

    auto nb = noiseBox.getLocalBounds().reduced(10); nb.removeFromTop(24);
    auto nrow = nb.removeFromTop(24);
    noiseDirect.setBounds(nrow.removeFromLeft(110));
    noiseLevel.setBounds(nb.removeFromTop(138).removeFromLeft(66));

    auto fb = filterBox.getLocalBounds().reduced(10); fb.removeFromTop(24);
    filterMode.setBounds(fb.removeFromTop(24).removeFromLeft(84));
    fb.removeFromTop(8);
    auto frow = fb.removeFromTop(154);
    for (auto* k : { &filterCutoff, &filterRes, &filterDrive, &filterMix }) { k->setBounds(frow.removeFromLeft(56).reduced(2)); frow.removeFromLeft(4); }

    auto bb = bassBox.getLocalBounds().reduced(10); bb.removeFromTop(24);
    auto brow1 = bb.removeFromTop(152);
    for (auto* k : { &reeseAmt, &warhornAmt, &wompAmt, &subharmAmt }) { k->setBounds(brow1.removeFromLeft(78).reduced(2)); brow1.removeFromLeft(4); }
    bb.removeFromTop(4);
    auto brow2 = bb.removeFromTop(152);
    for (auto* k : { &comboAmt, &stereoWidth, &fmAmt, &airAmt }) { k->setBounds(brow2.removeFromLeft(78).reduced(2)); brow2.removeFromLeft(4); }

    auto cb = charBox.getLocalBounds().reduced(10); cb.removeFromTop(24);
    auto crow = cb.removeFromTop(154);
    for (auto* k : { &bodyAmt, &growlAmt, &shimmerAmt, &motionAmt }) { k->setBounds(crow.removeFromLeft(56).reduced(2)); crow.removeFromLeft(4); }

    auto eb = envBox.getLocalBounds().reduced(10); eb.removeFromTop(24);
    auto erow = eb.removeFromTop(154);
    for (auto* k : { &env1A, &env1D, &env1S, &env1R }) { k->setBounds(erow.removeFromLeft(56).reduced(2)); erow.removeFromLeft(4); }

    auto lb = lfoBox.getLocalBounds().reduced(10); lb.removeFromTop(24);
    auto lrow = lb.removeFromTop(154);
    for (auto* k : { &lfo1Rate, &lfo1Amt, &lfo1Shape, &lfo1Smooth }) { k->setBounds(lrow.removeFromLeft(56).reduced(2)); lrow.removeFromLeft(4); }

    auto mb = matrixBox.getLocalBounds().reduced(10); mb.removeFromTop(24);
    auto viewArea = mb.removeFromRight(214);
    matrixView.setBounds(viewArea);
    ComboBox* srcs[] = { &mod1Src,&mod2Src,&mod3Src,&mod4Src,&mod5Src,&mod6Src };
    ComboBox* dsts[] = { &mod1Dst,&mod2Dst,&mod3Dst,&mod4Dst,&mod5Dst,&mod6Dst };
    SerumKnob* amts[] = { &mod1Amt,&mod2Amt,&mod3Amt,&mod4Amt,&mod5Amt,&mod6Amt };
    for (int i = 0; i < 6; ++i)
    {
        auto row = mb.removeFromTop(36);
        srcs[i]->setBounds(row.removeFromLeft(118)); row.removeFromLeft(6);
        dsts[i]->setBounds(row.removeFromLeft(140)); row.removeFromLeft(6);
        amts[i]->setBounds(row.removeFromLeft(78).withHeight(120));
        mb.removeFromTop(6);
    }

    auto xb = fxBox.getLocalBounds().reduced(10); xb.removeFromTop(24);
    auto xrow = xb.removeFromTop(154);
    for (auto* k : { &distDrive, &distMix, &chorusMix, &delayMix, &reverbMix, &compAmt }) { k->setBounds(xrow.removeFromLeft(46).reduced(2)); xrow.removeFromLeft(2); }

    auto sp = spaceBox.getLocalBounds().reduced(10); sp.removeFromTop(24);
    auto sprow = sp.removeFromTop(154);
    for (auto* k : { &reverbSize, &reverbTone, &chorusRate, &delayTime }) { k->setBounds(sprow.removeFromLeft(46).reduced(2)); sprow.removeFromLeft(2); }

    auto pb = perfBox.getLocalBounds().reduced(10); pb.removeFromTop(24);
    auto prow = pb.removeFromTop(24);
    playMode.setBounds(prow.removeFromLeft(102)); prow.removeFromLeft(8); polyphony.setBounds(prow.removeFromLeft(96));
    pb.removeFromTop(10);
    auto pkn = pb.removeFromTop(154);
    glide.setBounds(pkn.removeFromLeft(64).reduced(2)); pkn.removeFromLeft(4);
    masterGain.setBounds(pkn.removeFromLeft(64).reduced(2));
}
