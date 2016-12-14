/*
	QMPlay2 is a video and audio player.
	Copyright (C) 2010-2016  Błażej Szczygieł

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published
	by the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <AudioFilters.hpp>
#include <BS2B.hpp>
#include <Equalizer.hpp>
#include <EqualizerGUI.hpp>
#include <VoiceRemoval.hpp>
#include <PhaseReverse.hpp>
#include <Echo.hpp>
#include <DysonCompressor.hpp>

AudioFilters::AudioFilters() :
	Module("AudioFilters")
{
	moduleImg = QImage(":/AudioFilters");

	init("BS2B", false);

	init("Equalizer", false);
	int nbits = getInt("Equalizer/nbits");
	if (nbits < 8 || nbits > 16)
		set("Equalizer/nbits", 10);
	int count = getInt("Equalizer/count");
	if (count < 2 || count > 20)
		set("Equalizer/count", (count = 8));
	int minFreq = getInt("Equalizer/minFreq");
	if (minFreq < 10 || minFreq > 300)
		set("Equalizer/minFreq", (minFreq = 200));
	int maxFreq = getInt("Equalizer/maxFreq");
	if (maxFreq < 10000 || maxFreq > 96000)
		set("Equalizer/maxFreq", (maxFreq = 18000));
	init("Equalizer/-1", 50);
	for (int i = 0; i < count; ++i)
		init("Equalizer/" + QString::number(i), 50);

	init("VoiceRemoval", false);

	init("PhaseReverse", false);
	init("PhaseReverse/ReverseRight", false);

	init("Echo", false);
	init("Echo/Delay", 500);
	init("Echo/Volume", 50);
	init("Echo/Feedback", 50);
	init("Echo/Surround", false);

	init("Compressor", false);
	init("Compressor/PeakPercent", 90);
	init("Compressor/ReleaseTime", 0.2);
	init("Compressor/FastGainCompressionRatio", 0.9);
	init("Compressor/OverallCompressionRatio", 0.6);

	if (getBool("Equalizer"))
	{
		bool disableEQ = true;
		for (int i = -1; i < count; ++i)
			disableEQ &= getInt("Equalizer/" + QString::number(i)) == 50;
		if (disableEQ)
			set("Equalizer", false);
	}
}

QList<AudioFilters::Info> AudioFilters::getModulesInfo(const bool) const
{
	QList<Info> modulesInfo;
	modulesInfo += Info(BS2BName, AUDIOFILTER);
	modulesInfo += Info(EqualizerName, AUDIOFILTER);
	modulesInfo += Info(EqualizerGUIName, QMPLAY2EXTENSION);
	modulesInfo += Info(VoiceRemovalName, AUDIOFILTER);
	modulesInfo += Info(PhaseReverseName, AUDIOFILTER);
	modulesInfo += Info(EchoName, AUDIOFILTER);
	modulesInfo += Info(DysonCompressorName, AUDIOFILTER);
	return modulesInfo;
}
void *AudioFilters::createInstance(const QString &name)
{
	if (name == BS2BName)
		return new BS2B(*this);
	if (name == EqualizerName)
		return new Equalizer(*this);
	else if (name == EqualizerGUIName)
		return static_cast<QMPlay2Extensions *>(new EqualizerGUI(*this));
	else if (name == VoiceRemovalName)
		return new VoiceRemoval(*this);
	else if (name == PhaseReverseName)
		return new PhaseReverse(*this);
	else if (name == EchoName)
		return new Echo(*this);
	else if (name == DysonCompressorName)
		return new DysonCompressor(*this);
	return NULL;
}

AudioFilters::SettingsWidget *AudioFilters::getSettingsWidget()
{
	return new ModuleSettingsWidget(*this);
}

QMPLAY2_EXPORT_PLUGIN(AudioFilters)

/**/

#include <Slider.hpp>

#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>

ModuleSettingsWidget::ModuleSettingsWidget(Module &module) :
	Module::SettingsWidget(module)
{
	bs2bB = new QCheckBox(tr(BS2BName));
	bs2bB->setChecked(sets().getBool("BS2B"));
	connect(bs2bB, SIGNAL(clicked()), this, SLOT(bs2bToggle()));

	voiceRemovalEB = new QCheckBox(tr("Voice removal"));
	voiceRemovalEB->setChecked(sets().getBool("VoiceRemoval"));
	connect(voiceRemovalEB, SIGNAL(clicked()), this, SLOT(voiceRemovalToggle()));


	phaseReverseEB = new QCheckBox(tr("Phase reverse"));
	phaseReverseEB->setChecked(sets().getBool("PhaseReverse"));
	connect(phaseReverseEB, SIGNAL(clicked()), this, SLOT(phaseReverse()));

	phaseReverseRightB = new QCheckBox(tr("Reverse the right channel phase"));
	phaseReverseRightB->setChecked(sets().getBool("PhaseReverse/ReverseRight"));
	connect(phaseReverseRightB, SIGNAL(clicked()), this, SLOT(phaseReverse()));

	phaseReverseRightB->setEnabled(phaseReverseEB->isChecked());


	echoB = new QGroupBox(tr("Echo"));
	echoB->setCheckable(true);
	echoB->setChecked(sets().getBool("Echo"));
	connect(echoB, SIGNAL(clicked()), this, SLOT(echo()));

	echoDelayS = new Slider;
	echoDelayS->setRange(1, 1000);
	echoDelayS->setValue(sets().getUInt("Echo/Delay"));
	connect(echoDelayS, SIGNAL(valueChanged(int)), this, SLOT(echo()));

	echoVolumeS = new Slider;
	echoVolumeS->setRange(1, 100);
	echoVolumeS->setValue(sets().getUInt("Echo/Volume"));
	connect(echoVolumeS, SIGNAL(valueChanged(int)), this, SLOT(echo()));

	echoFeedbackS = new Slider;
	echoFeedbackS->setRange(1, 100);
	echoFeedbackS->setValue(sets().getUInt("Echo/Feedback"));
	connect(echoFeedbackS, SIGNAL(valueChanged(int)), this, SLOT(echo()));

	echoSurroundB = new QCheckBox(tr("Echo surround"));
	echoSurroundB->setChecked(sets().getBool("Echo/Surround"));
	connect(echoSurroundB, SIGNAL(clicked()), this, SLOT(echo()));

	QFormLayout *echoBLayout = new QFormLayout(echoB);
	echoBLayout->addRow(tr("Echo delay") + ": ", echoDelayS);
	echoBLayout->addRow(tr("Echo volume") + ": ", echoVolumeS);
	echoBLayout->addRow(tr("Echo repeat") + ": ", echoFeedbackS);
	echoBLayout->addRow(echoSurroundB);


	compressorB = new QGroupBox(tr("Dynamic range compression"));
	compressorB->setCheckable(true);
	compressorB->setChecked(sets().getBool("Compressor"));
	connect(compressorB, SIGNAL(clicked()), this, SLOT(compressor()));

	compressorPeakS = new Slider;
	compressorPeakS->setRange(1, 20);
	compressorPeakS->setValue(sets().getInt("Compressor/PeakPercent") / 5);
	connect(compressorPeakS, SIGNAL(valueChanged(int)), this, SLOT(compressor()));

	compressorReleaseTimeS = new Slider;
	compressorReleaseTimeS->setRange(1, 20);
	compressorReleaseTimeS->setValue(sets().getDouble("Compressor/ReleaseTime") * 20);
	connect(compressorReleaseTimeS, SIGNAL(valueChanged(int)), this, SLOT(compressor()));

	compressorFastRatioS = new Slider;
	compressorFastRatioS->setRange(1, 20);
	compressorFastRatioS->setValue(sets().getDouble("Compressor/FastGainCompressionRatio") * 20);
	connect(compressorFastRatioS, SIGNAL(valueChanged(int)), this, SLOT(compressor()));

	compressorRatioS = new Slider;
	compressorRatioS->setRange(1, 20);
	compressorRatioS->setValue(sets().getDouble("Compressor/OverallCompressionRatio") * 20);
	connect(compressorRatioS, SIGNAL(valueChanged(int)), this, SLOT(compressor()));

	QFormLayout *compressorBLayout = new QFormLayout(compressorB);
	compressorBLayout->addRow(tr("Peak limit") + ": ", compressorPeakS); //[%]
	compressorBLayout->addRow(tr("Release time") + ": ", compressorReleaseTimeS); //[s]
	compressorBLayout->addRow(tr("Fast compression ratio") + ": ", compressorFastRatioS);
	compressorBLayout->addRow(tr("Overall compression ratio") + ": ", compressorRatioS);


	QLabel *eqQualityL = new QLabel(tr("Sound equalizer quality") + ": ");

	eqQualityB = new QComboBox;
	eqQualityB->addItems(QStringList()
		<< tr("Low") + ", " + tr("filter size") + ": 256"
		<< tr("Low") + ", " + tr("filter size") + ": 512"
		<< tr("Medium") + ", " + tr("filter size") + ": 1024"
		<< tr("Medium") + ", " + tr("filter size") + ": 2048"
		<< tr("High") + ", " + tr("filter size") + ": 4096"
		<< tr("Very high") + ", " + tr("filter size") + ": 8192"
		<< tr("Very high") + ", " + tr("filter size") + ": 16384"
		<< tr("Very high") + ", " + tr("filter size") + ": 32768"
		<< tr("Very high") + ", " + tr("filter size") + ": 65536"
	);
	eqQualityB->setCurrentIndex(sets().getInt("Equalizer/nbits") - 8);

	QLabel *eqSlidersL = new QLabel(tr("Slider count in sound equalizer") + ": ");

	eqSlidersB = new QSpinBox;
	eqSlidersB->setRange(2, 20);
	eqSlidersB->setValue(sets().getInt("Equalizer/count"));

	eqMinFreqB = new QSpinBox;
	eqMinFreqB->setPrefix(tr("Minimum frequency") + ": ");
	eqMinFreqB->setSuffix(" Hz");
	eqMinFreqB->setRange(10, 300);
	eqMinFreqB->setValue(sets().getInt("Equalizer/minFreq"));

	eqMaxFreqB = new QSpinBox;
	eqMaxFreqB->setPrefix(tr("Maximum frequency") + ": ");
	eqMaxFreqB->setSuffix(" Hz");
	eqMaxFreqB->setRange(10000, 96000);
	eqMaxFreqB->setValue(sets().getInt("Equalizer/maxFreq"));

	QGridLayout *layout = new QGridLayout(this);
	layout->addWidget(bs2bB, 0, 0, 1, 2);
	layout->addWidget(voiceRemovalEB, 1, 0, 1, 2);
	layout->addWidget(phaseReverseEB, 2, 0, 1, 2);
	layout->addWidget(phaseReverseRightB, 3, 0, 1, 2);
	layout->addWidget(echoB, 4, 0, 1, 2);
	layout->addWidget(compressorB, 5, 0, 1, 2);
	layout->addWidget(eqQualityL, 6, 0, 1, 1);
	layout->addWidget(eqQualityB, 6, 1, 1, 1);
	layout->addWidget(eqSlidersL, 7, 0, 1, 1);
	layout->addWidget(eqSlidersB, 7, 1, 1, 1);
	layout->addWidget(eqMinFreqB, 8, 0, 1, 1);
	layout->addWidget(eqMaxFreqB, 8, 1, 1, 1);
}

void ModuleSettingsWidget::bs2bToggle()
{
	sets().set("BS2B", bs2bB->isChecked());
	SetInstance<BS2B>();
}
void ModuleSettingsWidget::voiceRemovalToggle()
{
	sets().set("VoiceRemoval", voiceRemovalEB->isChecked());
	SetInstance<VoiceRemoval>();
}
void ModuleSettingsWidget::phaseReverse()
{
	sets().set("PhaseReverse", phaseReverseEB->isChecked());
	sets().set("PhaseReverse/ReverseRight", phaseReverseRightB->isChecked());
	phaseReverseRightB->setEnabled(phaseReverseEB->isChecked());
	SetInstance<PhaseReverse>();
}
void ModuleSettingsWidget::echo()
{
	sets().set("Echo", echoB->isChecked());
	sets().set("Echo/Delay", echoDelayS->value());
	sets().set("Echo/Volume", echoVolumeS->value());
	sets().set("Echo/Feedback", echoFeedbackS->value());
	sets().set("Echo/Surround", echoSurroundB->isChecked());
	SetInstance<Echo>();
}
void ModuleSettingsWidget::compressor()
{
	sets().set("Compressor", compressorB->isChecked());
	sets().set("Compressor/PeakPercent", compressorPeakS->value() * 5);
	sets().set("Compressor/ReleaseTime", compressorReleaseTimeS->value() / 20.0);
	sets().set("Compressor/FastGainCompressionRatio", compressorFastRatioS->value() / 20.0);
	sets().set("Compressor/OverallCompressionRatio", compressorRatioS->value() / 20.0);
	SetInstance<DysonCompressor>();
}

void ModuleSettingsWidget::saveSettings()
{
	sets().set("Equalizer/nbits", eqQualityB->currentIndex() + 8);
	sets().set("Equalizer/count", eqSlidersB->value());
	sets().set("Equalizer/minFreq", eqMinFreqB->value());
	sets().set("Equalizer/maxFreq", eqMaxFreqB->value());
}
