





static int wm8960_set_dai_sysclk(struct snd_soc_dai *codec_dai,int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_component *component = codec_dai->component;
	struct clk *mclk;
	struct clk *pll;

	pll = devm_clk_get(component->dev, "pll");
	if (IS_ERR(pll))
		return PTR_ERR(pll);

	mclk = clk_get_parent(pll);

	return clk_set_rate(mclk, freq);
}


static int wm8960_set_dai_fmt(struct snd_soc_dai *codec_dai, unsigned int fmt)
{
	struct snd_soc_component *component = codec_dai->component;
	u8 iface_reg_1 = 0;
	u8 iface_reg_2 = 0;
	u8 iface_reg_3 = 0;

	/* set master/slave audio interface */
	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		iface_reg_1 |= WM8960_BCLKMASTER | WM8960_WCLKMASTER;
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		break;
	default:
		printk(KERN_ERR "wm8960: invalid DAI master/slave interface\n");
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		break;
	case SND_SOC_DAIFMT_DSP_A:
		iface_reg_1 |= (WM8960_DSP_MODE <<
				WM8960_IFACE1_DATATYPE_SHIFT);
		iface_reg_3 |= WM8960_BCLKINV_MASK; /* invert bit clock */
		iface_reg_2 = 0x01; /* add offset 1 */
		break;
	case SND_SOC_DAIFMT_DSP_B:
		iface_reg_1 |= (WM8960_DSP_MODE <<
				WM8960_IFACE1_DATATYPE_SHIFT);
		iface_reg_3 |= WM8960_BCLKINV_MASK; /* invert bit clock */
		break;
	case SND_SOC_DAIFMT_RIGHT_J:
		iface_reg_1 |= (WM8960_RIGHT_JUSTIFIED_MODE <<
				WM8960_IFACE1_DATATYPE_SHIFT);
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		iface_reg_1 |= (WM8960_LEFT_JUSTIFIED_MODE <<
				WM8960_IFACE1_DATATYPE_SHIFT);
		break;
	default:
		printk(KERN_ERR "wm8960: invalid DAI interface format\n");
		return -EINVAL;
	}

	snd_soc_component_update_bits(component, WM8960_IFACE1,
				WM8960_IFACE1_DATATYPE_MASK |
				WM8960_IFACE1_MASTER_MASK, iface_reg_1);
	snd_soc_component_update_bits(component, WM8960_IFACE2,
				WM8960_DATA_OFFSET_MASK, iface_reg_2);
	snd_soc_component_update_bits(component, WM8960_IFACE3,
				WM8960_BCLKINV_MASK, iface_reg_3);

	return 0;
}


static int wm8960_hw_params(struct snd_pcm_substream *substream,struct snd_pcm_hw_params *params,struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
	struct wm8960_priv *wm8960 = snd_soc_component_get_drvdata(component);
	u8 iface1_reg = 0;
	u8 dacsetup_reg = 0;

	wm8960_setup_clocks(component, params_rate(params),
			     params_channels(params),
			     params_physical_width(params));

	switch (params_physical_width(params)) {
	case 16:
		iface1_reg |= (WM8960_WORD_LEN_16BITS <<
				   WM8960_IFACE1_DATALEN_SHIFT);
		break;
	case 20:
		iface1_reg |= (WM8960_WORD_LEN_20BITS <<
				   WM8960_IFACE1_DATALEN_SHIFT);
		break;
	case 24:
		iface1_reg |= (WM8960_WORD_LEN_24BITS <<
				   WM8960_IFACE1_DATALEN_SHIFT);
		break;
	case 32:
		iface1_reg |= (WM8960_WORD_LEN_32BITS <<
				   WM8960_IFACE1_DATALEN_SHIFT);
		break;
	}
	snd_soc_component_update_bits(component, WM8960_IFACE1,
				WM8960_IFACE1_DATALEN_MASK, iface1_reg);

	if (params_channels(params) == 1) {
		dacsetup_reg = WM8960_RDAC2LCHN | WM8960_LDAC2LCHN;
	} else {
		if (wm8960->swapdacs)
			dacsetup_reg = WM8960_RDAC2LCHN | WM8960_LDAC2RCHN;
		else
			dacsetup_reg = WM8960_LDAC2LCHN | WM8960_RDAC2RCHN;
	}
	snd_soc_component_update_bits(component, WM8960_DACSETUP,
				WM8960_DAC_CHAN_MASK, dacsetup_reg);

	return 0;
}

static int wm8960_mute(struct snd_soc_dai *dai, int mute, int direction)
{
	struct snd_soc_component *component = dai->component;

	snd_soc_component_update_bits(component, WM8960_DACMUTE,
				WM8960_MUTEON, mute ? WM8960_MUTEON : 0);

	return 0;
}


#define WM8960_RATES	SNDRV_PCM_RATE_8000_192000
#define WM8960_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FMTBIT_S20_3LE \
			 | SNDRV_PCM_FMTBIT_S24_LE | SNDRV_PCM_FMTBIT_S24_3LE \
			 | SNDRV_PCM_FMTBIT_S32_LE)


//	***************************************************************

static const struct snd_soc_dai_ops wm8960_ops = {
	.hw_params = wm8960_hw_params,
	.mute_stream = wm8960_mute,
	.set_fmt = wm8960_set_dai_fmt,
	.set_sysclk = wm8960_set_dai_sysclk,
	.no_capture_mute = 1,
};

static struct snd_soc_dai_driver wm8960_dai = {
	.name = "wm8960",
	.playback = {
			 .stream_name = "Playback",
			 .channels_min = 1,
			 .channels_max = 2,
			 .rates = WM8960_RATES,
			 .formats = WM8960_FORMATS,},
	.capture = {
			.stream_name = "Capture",
			.channels_min = 1,
			.channels_max = 8,
			.rates = WM8960_RATES,
			.formats = WM8960_FORMATS,},
	.ops = &wm8960_ops,
	.symmetric_rate = 1,
};


