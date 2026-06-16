#include "PluginEditor.h"
#include "BinaryData.h"

// ===== CustomKnob =====

CustomKnob::CustomKnob(juce::RangedAudioParameter& p, const juce::String& lbl, juce::Colour accentCol)
    : param(p), label(lbl), accent(accentCol) {}

float CustomKnob::getNormalised() const
{
    if (auto* fp = dynamic_cast<juce::AudioParameterFloat*>(&param))
    {
        auto r = fp->getNormalisableRange();
        return (fp->get() - r.start) / (r.end - r.start);
    }
    return 0.0f;
}

void CustomKnob::setNormalised(float v)
{
    v = juce::jlimit(0.0f, 1.0f, v);
    if (auto* fp = dynamic_cast<juce::AudioParameterFloat*>(&param))
    {
        auto r = fp->getNormalisableRange();
        *fp = r.start + v * (r.end - r.start);
    }
}

juce::String CustomKnob::getDisplayText() const
{
    if (auto* fp = dynamic_cast<juce::AudioParameterFloat*>(&param))
        return juce::String(fp->get(), 1);
    return {};
}

void CustomKnob::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    auto sz = juce::jmin(b.getWidth(), b.getHeight());
    auto knob = juce::Rectangle<float>(0, 0, sz * 0.78f, sz * 0.78f).withCentre(b.getCentre().withY(b.getCentreY() - 8));
    auto arc = knob.reduced(sz * 0.07f);

    g.setColour(juce::Colours::black.withAlpha(0.9f));
    g.fillEllipse(knob);
    g.setColour(accent.withAlpha(0.4f));
    g.drawEllipse(knob, 1.5f);
    g.setColour(juce::Colours::white.withAlpha(0.12f));
    g.drawEllipse(arc, 1.5f);

    auto startA = juce::MathConstants<float>::pi * 0.75f;
    auto endA   = juce::MathConstants<float>::pi * 2.25f;
    auto angle  = startA + normalised * (endA - startA);

    if (normalised > 0.001f)
    {
        juce::Path p;
        p.addArc(arc.getX(), arc.getY(), arc.getWidth(), arc.getHeight(), startA, angle, true);
        g.setColour(accent);
        g.strokePath(p, juce::PathStrokeType(2.5f));
    }

    auto nl = arc.getWidth() * 0.35f;
    auto nx = b.getCentreX() + std::cos(angle) * nl;
    auto ny = knob.getCentreY() + std::sin(angle) * nl;
    g.setColour(accent.brighter(0.4f));
    g.drawLine(b.getCentreX(), knob.getCentreY(), nx, ny, 2.0f);
    g.fillEllipse(nx - 3, ny - 3, 6, 6);

    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    g.drawText(label, b.withY(knob.getBottom() + 12).withHeight(14), juce::Justification::centred);
}

void CustomKnob::mouseDown(const juce::MouseEvent& e)
{
    isDragging = true;
    dragStartY = e.position.y;
    dragStartNorm = normalised;
}

void CustomKnob::mouseDrag(const juce::MouseEvent& e)
{
    if (!isDragging) return;
    normalised = juce::jlimit(0.0f, 1.0f, dragStartNorm + (dragStartY - e.position.y) / 120.0f);
    setNormalised(normalised);
    repaint();
}

void CustomKnob::updateValue()
{
    normalised = getNormalised();
    repaint();
}

// ===== SyncButton =====

SyncButton::SyncButton(juce::AudioParameterBool& p) : Button("Sync"), param(p)
{
    setClickingTogglesState(true);
    setToggleState(param.get(), juce::dontSendNotification);
}

void SyncButton::paintButton(juce::Graphics& g, bool, bool)
{
    auto b = getLocalBounds().toFloat();
    auto on = getToggleState();
    g.setColour(on ? juce::Colour(0xff4d8a4d) : juce::Colour(0xff222222));
    g.fillRoundedRectangle(b, 6);
    g.setColour(on ? juce::Colour(0xff7ccc7c) : juce::Colour(0xff555555));
    g.drawRoundedRectangle(b, 6, 1.5f);
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(juce::FontOptions(14.0f, juce::Font::bold)));
    g.drawText("SYNC", b, juce::Justification::centred);
}

void SyncButton::clicked()
{
    param = getToggleState();
}

// ===== NoteSelector =====

NoteSelector::NoteSelector(juce::AudioParameterChoice& p) : ComboBox("Note"), param(p)
{
    for (int i = 0; i < p.choices.size(); ++i)
        addItem(p.choices[i], i + 1);
    setSelectedItemIndex(param.getIndex(), juce::dontSendNotification);
    setJustificationType(juce::Justification::centred);
    onChange = [this] {
        param = static_cast<float>(getSelectedItemIndex()) / static_cast<float>(param.choices.size() - 1);
    };
}

NoteSelector::~NoteSelector() = default;

// ===== TapTempoButton =====

TapTempoButton::TapTempoButton(int idx, VSTPluginAudioProcessor& proc)
    : index(idx), processor(proc) {}

void TapTempoButton::paint(juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    auto now = juce::Time::getMillisecondCounter();
    if (lastFlash > 0 && now - lastFlash < 250)
        flashAlpha = juce::jmax(0.0f, 1.0f - static_cast<float>(now - lastFlash) / 250.0f);

    g.setColour(juce::Colour(0xff3a2818));
    g.fillRoundedRectangle(b, 4);
    if (flashAlpha > 0.0f)
    {
        g.setColour(juce::Colour(0xfffcca46).withAlpha(flashAlpha * 0.4f));
        g.fillRoundedRectangle(b, 4);
    }
    g.setColour(juce::Colour(0xff7c5c33));
    g.drawRoundedRectangle(b, 4, 1.5f);
    g.setColour(juce::Colour(0xfffcca46));
    g.setFont(juce::Font(juce::FontOptions(16.0f, juce::Font::bold)));
    g.drawText("TAP", b, juce::Justification::centred);
}

void TapTempoButton::mouseDown(const juce::MouseEvent&)
{
    flashAlpha = 1.0f;
    lastFlash = juce::Time::getMillisecondCounter();
    processor.tapTempo(index);
    repaint();
}

void TapTempoButton::updateBpm() { repaint(); }

// ===== Editor =====

VSTPluginAudioProcessorEditor::VSTPluginAudioProcessorEditor(VSTPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    auto imgData = BinaryData::bg_png;
    auto imgSize = BinaryData::bg_pngSize;
    if (imgData != nullptr && imgSize > 0)
        background = juce::ImageCache::getFromMemory(imgData, (int)imgSize);

    auto green = juce::Colour(0xff6aaa6a);
    auto green2 = juce::Colour(0xff8fcc8f);
    auto red = juce::Colour(0xffd64545);

    auto addKnob = [&](juce::RangedAudioParameter& pr, const juce::String& lb, juce::Colour c)
    {
        auto k = std::make_unique<CustomKnob>(pr, lb, c);
        addAndMakeVisible(*k);
        return k;
    };

    dryWetKnob = addKnob(*processorRef.dryWetParam, "Mix", green);

    for (int i = 0; i < 3; ++i)
    {
        auto& ui = tapUI[i];
        ui.tapBtn = std::make_unique<TapTempoButton>(i, processorRef);
        addAndMakeVisible(*ui.tapBtn);
        ui.syncBtn = std::make_unique<SyncButton>(*processorRef.tap[i].sync);
        addAndMakeVisible(*ui.syncBtn);
        ui.noteCombo = std::make_unique<NoteSelector>(*processorRef.tap[i].noteDiv);
        addAndMakeVisible(*ui.noteCombo);
        ui.timeKnob   = addKnob(*processorRef.tap[i].timeMs,   "Time",     green);
        ui.fbKnob     = addKnob(*processorRef.tap[i].feedback, "Feedback", green);
        ui.filterKnob = addKnob(*processorRef.tap[i].filter,   "Filter",   green);
    }

    modRateKnob   = addKnob(*processorRef.modRateParam,   "Rate",  green);
    modDepthKnob  = addKnob(*processorRef.modDepthParam,  "Depth", green);
    distDriveKnob = addKnob(*processorRef.distDriveParam, "Drive", red);
    distBlendKnob = addKnob(*processorRef.distBlendParam, "Blend", red);

    setSize(1300, 900);
    setResizable(true, true);
    startTimerHz(20);
}

VSTPluginAudioProcessorEditor::~VSTPluginAudioProcessorEditor() { stopTimer(); }

void VSTPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    if (background.isValid())
        g.drawImage(background, getLocalBounds().toFloat());
    else
        g.fillAll(juce::Colour(0xff2e2e2e));

    // Zonas de sección (semi-transparentes sobre la imagen)
    auto drawZone = [&](juce::Rectangle<int> r, juce::Colour c, const juce::String& title)
    {
        g.setColour(juce::Colour(0xff1a1a1a).withAlpha(0.65f));
        g.fillRoundedRectangle(r.toFloat(), 8);
        g.setColour(c.withAlpha(0.5f));
        g.drawRoundedRectangle(r.toFloat(), 8, 1.5f);
        g.setColour(c);
        g.setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::bold)));
        g.drawText(title, r.getX() + 14, r.getY() - 18, 600, 16, juce::Justification::centredLeft);
    };

    drawZone(juce::Rectangle<int>(550, 240, 230, 150), juce::Colour(0xfffcca46), "MIX");
    drawZone(juce::Rectangle<int>(30, 380, 1240, 360), juce::Colour(0xff6aaa6a), "TAPS — TIME / FEEDBACK / FILTER");
    drawZone(juce::Rectangle<int>(140, 740, 380, 150), juce::Colour(0xff6aaa6a), "MODULATION — LFO RATE & DEPTH");
    drawZone(juce::Rectangle<int>(700, 740, 420, 150), juce::Colour(0xffd64545), "DISTORTION — PARALLEL DRIVE & BLEND");
}

// Todas las posiciones en píxeles, ajustadas a ventana 1300x900 (escala desde 1451x1084 ≈ 0.896 / 0.831)
void VSTPluginAudioProcessorEditor::resized()
{
    // Mix knob (centro del banner superior)
    dryWetKnob->setBounds(640, 270, 60, 90);

    // ===== Tap 1 =====
    tapUI[0].tapBtn->setBounds(200, 400, 90, 35);
    tapUI[0].syncBtn->setBounds(50, 470, 130, 50);
    tapUI[0].noteCombo->setBounds(195, 470, 220, 50);
    tapUI[0].timeKnob->setBounds(75, 600, 80, 100);
    tapUI[0].fbKnob->setBounds(195, 600, 80, 100);
    tapUI[0].filterKnob->setBounds(320, 600, 80, 100);

    // ===== Tap 2 =====
    tapUI[1].tapBtn->setBounds(620, 400, 90, 35);
    tapUI[1].syncBtn->setBounds(465, 470, 130, 50);
    tapUI[1].noteCombo->setBounds(610, 470, 220, 50);
    tapUI[1].timeKnob->setBounds(490, 600, 80, 100);
    tapUI[1].fbKnob->setBounds(610, 600, 80, 100);
    tapUI[1].filterKnob->setBounds(730, 600, 80, 100);

    // ===== Tap 3 =====
    tapUI[2].tapBtn->setBounds(1030, 400, 90, 35);
    tapUI[2].syncBtn->setBounds(875, 470, 130, 50);
    tapUI[2].noteCombo->setBounds(1020, 470, 220, 50);
    tapUI[2].timeKnob->setBounds(900, 600, 80, 100);
    tapUI[2].fbKnob->setBounds(1020, 600, 80, 100);
    tapUI[2].filterKnob->setBounds(1140, 600, 80, 100);

    // ===== Modulación =====
    modRateKnob->setBounds(190, 760, 80, 100);
    modDepthKnob->setBounds(440, 760, 80, 100);

    // ===== Distorsión =====
    distDriveKnob->setBounds(770, 760, 80, 100);
    distBlendKnob->setBounds(1010, 760, 80, 100);
}

void VSTPluginAudioProcessorEditor::timerCallback()
{
    dryWetKnob->updateValue();
    for (int i = 0; i < 3; ++i)
    {
        tapUI[i].tapBtn->updateBpm();
        tapUI[i].timeKnob->updateValue();
        tapUI[i].fbKnob->updateValue();
        tapUI[i].filterKnob->updateValue();
    }
    modRateKnob->updateValue();
    modDepthKnob->updateValue();
    distDriveKnob->updateValue();
    distBlendKnob->updateValue();
}
