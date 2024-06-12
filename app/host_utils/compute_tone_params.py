# Copyright 2024 NXP
# SPDX-License-Identifier: BSD-3-Clause
####################################################################
# (python 2.7)
# plot freq spectrum from 2cmp binray capture 
# python compute_tone_params.py -f 61440000 -i C:\Temp\iqdata.bin
#


import sys
import os
import matplotlib.pyplot as plt
#from mpl_interaction import PanAndZoom
from numpy.fft import fft, fftshift
import matplotlib
import numpy as np
import scipy.fftpack
from fractions import gcd
import json
import struct
import csv
import copy
import math
# import codecs
import warnings
import getopt

slot_samples = [15408, 15344, 15344, 15344]
# This is due to the fixed 1024 samples dma size in VSPA. There will always be data from previous slot in the first
# buffer for slot 1, 2 and 3
# Ex: 30816 = 30*1024 + 96. The rest of the 928 samples are from slot 1. So in first buffer of slot 1 there will be 96
# samples form slot 0 and so on
slot_start_extra_samples = [0, 96, 64, 32]
slot_stop_extra_samples = [928, 960, 992, 0]
long_symb = 136+1024
short_symb = 72+1024
INV_AXI_WIDTH = 64  # bytes, 512 bits, 16 samples
MIN_DMA_SIZE = 27392  # bytes, 6848 samples (Long slot duration / 36)
MAX_DMA_SIZE = 37120  # bytes, 9280 samples (Long symbol duration / 2)

class MemDescriptor:
    def __init__(self, physical, virtual, size, default_align=64):
        self.physical = physical
        self.virtual = virtual
        self.size = size
        self.default_align = default_align
        self.offset = 0

class MemMap:
    FRAM = MemDescriptor(physical=0x4c40300000, virtual=0x300000,   size=512*1024)
    DDR = MemDescriptor(physical=0x81F0000000, virtual=0xF0000000, size=256*1024*1024)
    HALF_DDR = MemDescriptor(physical=0x81F8000000, virtual=0xF8000000, size=128*1024*1024)
    VSPA_MEM = [MemDescriptor(physical=0xA069001000 + i * 0x400000, virtual=0x1000, size=128*1024) for i in range(8)]
    HRAM = MemDescriptor(physical=0xA06D000000, virtual=0xE5000000, size=0x600000)  # 6 MB
    FRAMINV = MemDescriptor(physical=0xA06E000000, virtual=0xE6000000, size=0xC0000)  # 768 KB
    PEB = MemDescriptor(physical=0xA068200000, virtual=0xE0200000, size=0x200000)

class RfValidationTool:
    def __init__(self,fs=122880000*2):
			
        #upsamp = fs/122880000
        #self.sample_rate=upsamp*122880000
        #self.rx_sample_rate=upsamp*122880000

        self.sample_rate=fs
        self.rx_sample_rate=fs
 
        self.length = 0
        self.dma_size = 0
        self.max_amplitude = 0.5
        self.max_err = 0.01
        self.max_err_overflow = 0.05
        self.frequency = []
        self.amplitude = []
        print('Sampling Rate RX ' + str(self.rx_sample_rate/1000000) + 'MSPS')

    def compute_tone_params(self, ifile, tone_type, frequency_list, save_plot=''):
        figures = []

        sine_fft = 0
        fft_window = 8192
        signal_power = 0
        noise_power = 0
        noise_fft = 0
        rssi = 0
        samples = []
        max_amp_db = []
            
        frequency = frequency_list

        print('fft_window ' + str(fft_window) )
            
        logging_txt = open(ifile + ".csv", 'wb')

        # Parsing the captured bin file
        print('Input file is "', ifile)
        with open(ifile, 'rb') as f:
            byte = f.read(4)
            while byte != '':
                temp = struct.unpack('<hh', byte)
                lst = list(temp)
                temp = tuple(lst)
                samples.append(complex(temp[0], temp[1]) / (2 ** 15))
                byte = f.read(4)

        print('read ' + str(len(samples)) + ' samples (' + str((float(len(samples))*1000)/self.rx_sample_rate) + 'ms)' )

        # Time domain plot
        figures.append({'x': np.arange(0, len(samples) / float(self.rx_sample_rate),
                                       1 / float(self.rx_sample_rate)) * 10**6,
                        'y': np.real(samples),
                        'title': 'Time domain I samples ' ,
                        'xlabel': 'time [us]',
                        'ylabel': 'Amplitude','save_plot' : save_plot})

        figures.append({'x': np.arange(0, len(samples) / float(self.rx_sample_rate),
                                       1 / float(self.rx_sample_rate)) * 10**6,
                        'y': np.imag(samples),
                        'title': 'Time domain Q samples ' ,
                        'xlabel': 'time [us]',
                        'ylabel': 'Amplitude','save_plot' : save_plot})

        # PSD calculation
        for i in range(0, len(samples)):
            rssi = rssi + np.real(samples[i])**2 + np.imag(samples[i])**2
        rssi = 10 * np.log10(rssi/len(samples) + 10**(-16))
        print(' ')
        print('Received Signal')
        print ("  RSSI : " + str(round(rssi, 4)) + 'dBFS')

        # FFT calculation
        samp_arr = [samples[x:x + fft_window] for x in xrange(0, len(samples), fft_window)]
        for i in xrange(0, len(samp_arr)):
            if len(samp_arr[i]) == fft_window:
                sine_fft += np.abs(fft(samp_arr[i]))

        sine_fft = sine_fft / fft_window

        x_samples = np.arange(0, self.rx_sample_rate, self.rx_sample_rate/fft_window)
        sine_fft_shift = 0
        for i in xrange(0, len(samp_arr)):
            if len(samp_arr[i]) == fft_window:
                sine_fft_shift += np.abs(fftshift(fft(samp_arr[i])))
        sine_fft_shift = sine_fft_shift / fft_window
        sine_fft_shift = sine_fft_shift / (len(samples) / fft_window)
        x_fft_shift = np.arange(-self.rx_sample_rate/2, self.rx_sample_rate/2, self.rx_sample_rate/fft_window)
        figures.append({'x': x_fft_shift / float((10**6)),
                        'y': 20 * np.log10(sine_fft_shift + 10**(-16)),
                        'title': 'Frequency spectrum ',
                        'xlabel': 'Frequency [Mhz]',
                        'ylabel': 'Amplitude','save_plot' : save_plot})

        # PSD calculation
        psd = (sine_fft/(len(samples) / fft_window)) ** 2
        figures.append({'x': x_samples[0:len(psd)] / float(10**6),
                        'y': 10 * np.log10(psd[0:len(psd)] + 10**(-16)),
                        'title': 'PSD ' ,
                        'xlabel': 'Frequency[MHz]',
                        'ylabel': 'PSD[dBFS]','save_plot' : save_plot})
        if tone_type == 'sine':
            sine_fft = (2 * sine_fft / (len(samples) / fft_window))[0:len(sine_fft) / 2]
        elif tone_type == 'complex_sinusoid':
            sine_fft = (sine_fft / (len(samples) / fft_window))

        for x in range(0, len(sine_fft)):
            noise_fft += sine_fft[x]
            noise_power += psd[x]
        noise_power = noise_power / (len(sine_fft))
        print ("  Noise power:  " + str(round(10 * np.log10(noise_power + 10**(-16)), 2)) + "dBFS")

        # Amplitude(dB) calculation
        for i in range(0, len(frequency)):
            print(' ')
            print( '' + tone_type + ' at ' + str(frequency[i]/1000000) + 'MHz')
            signal_power=0
            freq_position = int(round(frequency[i] * fft_window / float(self.rx_sample_rate)))
            max_amp_db.append(20 * np.log10(sine_fft[freq_position] + 10**(-16)))
            logging_txt.write(str(int(frequency[i])) + ',' + str(round(max_amp_db[i], 4)) + ',' + str(1) + '\n')
            logging_txt.flush()
            print ("  Amplitude of " + str(frequency[i] / 1000000) + "MHz : " + str(round(max_amp_db[i], 4)) + "dBFS")
            # Signal power calculation
            if tone_type == 'sine':
                signal_power += 2 * psd[freq_position]
                print ("  Power of     " + str(frequency[i] / 1000000) + "MHz : " +
                       str(round(10*np.log10(2 * psd[freq_position] + 10**(-16)), 4)) + "dBFS")
            elif tone_type == 'complex_sinusoid':
                signal_power += psd[freq_position]
                print ("  Power of " + str(frequency[i] / 1000000) + "MHz : " +
                       str(round(10*np.log10(psd[freq_position] + 10**(-16)), 4)) + "dBFS")
            print ("  Signal Power: " + str(round(10 * np.log10(signal_power + 10**(-16)), 2)) + "dBFS")
            # SNR(dB) calculation
            snr = round(10 * np.log10(signal_power + 10**(-16)), 2) - round(10 * np.log10(noise_power + 10**(-16)), 2)
            print ("  SNR: " + str(round(snr, 2)) + "dBFS")

            # Noise power calculation
            if freq_position - 5 < 0:
                sine_fft[0:freq_position + 5] = 0
                psd[0:freq_position + 5] = 0
            elif freq_position + 5 > fft_window/2:
                sine_fft[freq_position - 5:fft_window/2] = 0
                psd[freq_position - 5:fft_window/2] = 0
            else:
                sine_fft[freq_position - 5:freq_position + 5] = 0
                psd[freq_position - 5:freq_position + 5] = 0
            sine_fft[freq_position] = 0
            psd[freq_position] = 0
            
        # Amplitude difference calculation between two tones
        if len(frequency) > 1:
            # Plot Frequency vs Amplitude
            fh = open(ifile + ".csv", 'r')
            reader = list(csv.reader(fh))
            temp1 = {}
            for r in reader:
                if str(r[0]) in temp1.keys():
                    temp1[str(r[0])][0] = float(temp1[str(r[0])][0]) + float(r[1])
                    temp1[str(r[0])][1] = temp1[str(r[0])][1] + 1
                else:
                    temp1[str(r[0])] = [(float(r[1]))]
                    temp1[str(r[0])].append(int(r[2]))
            x = []
            y = []
            for key in temp1.keys():
                x.append(int(key))
                y.append(float(temp1[key][0]) / float(temp1[key][1]))
            sorted_indexes = np.argsort(x)
            x.sort()
            sorted_y = []
            for i in range(len(sorted_indexes)):
                sorted_y.append(y[sorted_indexes[i]])
            figures.append({'x': [i / float((10**6)) for i in x],
                            'y': sorted_y,
                            'title': 'Amplitude variation over frequency statistic ' ,
                            'xlabel': 'Frequency[MHz]',
                            'ylabel': 'Amplitude[dBFS]','save_plot' : save_plot})
        print(' ')
        return figures

def plot_figures(figures_list):
    ax = None
    for idx, figure in enumerate(figures_list):
        # plt.figure(figure['title'])
        if idx == 0 or (figure['title'] != figures_list[idx-1]['title']):
            fig, ax = plt.subplots(num=figure['title'])
        #print figure['title']
        #print len(figure['x']), len(figure['y'])
        data = ax.plot(figure['x'], figure['y'])
        #plt.plot(figure['x'], figure['y'])
        #if len(figure['x']) < 491520:
            # activate cursors only if there are at most 16 slots
            #datacursor(data, hide_button=1, xytext=(30, 15))
        plt.title(figure['title'])
        plt.xlabel(figure['xlabel'])
        plt.ylabel(figure['ylabel'])
        plt.grid(True, linestyle='--')
        
        if 'time' in figure['xlabel']:
            plt.ylim(-1, 1)
        figure_copy = copy.deepcopy(figure)
        for key in ['x', 'y', 'title', 'xlabel', 'ylabel', 'slot_start_count', 'fs', 'save_plot']:
            try:
                figure_copy.pop(key)
            except KeyError:
                pass
    plt.show()
    plt.close('all')
    # threaded_print('Figures closed')


def main(argv):
   inputfile = './waveforms/tone0.bin'
   fs=491520000/2
   freq=[]
   try:
      opts, args = getopt.getopt(argv,"hi:o:f:w:r:",["ifile=","ofile=","fs=","ws="])
   except getopt.GetoptError:
      print ('compute_tone_params.py -i <inputfile> -f <samplig rate> -w <single tone sinus freq>')
      print ('python compute_tone_params.py -i waveforms_gnodeb\tone1.bin -f 491520000 -w 50000000 -w 10000000')
      sys.exit(2)
   for opt, arg in opts:
      if opt == '-h':
         print ('compute_tone_params.py -i <inputfile> -f <samplig rate> -w <single tone sinus freq>')
         print ('python compute_tone_params.py -i waveforms\\tone1.bin -f 491520000 -w 50000000 -w 10000000')
         sys.exit()
      elif opt in ("-i", "--ifile"):
         inputfile = arg
      elif opt in ("-f", "--fs"):
         fs = int(arg)
      elif opt in ("-w", "--ws"):
         freq.append(int(arg))
  
 
   tone_type = 'complex_sinusoid'
   rf_val = RfValidationTool(fs)
   figures = rf_val.compute_tone_params(inputfile,tone_type, freq)
   plot_figures(figures)

if __name__ == '__main__':
   main(sys.argv[1:])