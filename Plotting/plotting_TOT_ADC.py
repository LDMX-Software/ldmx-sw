# plot CHARGE vs ADC/TOT using Voltage_ADC_TOT.txt 
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator

plt.rc('font', family='sans-serif')

# Load the data from the text file
data = np.loadtxt('Voltage_ADC_TOT.txt')

# Apply the masks to filter the data
maskADC = (data[:, 0] < 60000) & (data[:, 1] != 0)
maskTOT = (data[:, 0] < 60000) & (data[:, 2] != 0)

filtered_data_ADC = data[maskADC]
filtered_data_TOT = data[maskTOT]


# IF YOU ARE CHANGING THE CAPACITANCE IN hcal_hardcoded_conditions CHANGE IT HERE AS WELL!!
capacitance = 13/5.1 

# Extract the columns
x_adc = capacitance * filtered_data_ADC[:, 0] * 1e-3  # pC
y_adc = filtered_data_ADC[:, 1]

x_tot = capacitance * filtered_data_TOT[:, 0] * 1e-3  # pC
y_tot = filtered_data_TOT[:, 2]

# Create a figure
fig, ax1 = plt.subplots(figsize=(12, 7), dpi=300)

# Plot the ADC data points
ax1.plot(x_adc, y_adc, '.', color='darkviolet', label='ADC')

# Plot the TOT data points
ax1.plot(x_tot, y_tot, '.', color='deeppink', label='TOT')

ax1.set_xlabel('Charge [pC]')
ax1.set_ylabel('adc_count / tot_count')
ax1.grid(True, linestyle = '--', alpha = 0.3)
ax1.legend(loc='upper right', bbox_to_anchor=(1,0.9), frameon = False)
#ax1.legend(loc='upper right')
ax1.set_xticks(range(0,50,5))
ax1.set_yticks(range(0, 1200, 100))


# Plot multiple x-axes
x = np.arange(0,120,1)

# Functions for conversions
def pC2MIP(xpc):
    return xpc * (15 / 13.0)

def pC2MeV(ypc):
    return ypc * (15 * 4.66 / 13.0)

def pC2PE(zpc):
    return zpc * (15 * 68 / 13.0)

# 2nd x-axis
ax2 = ax1.twiny()
xlim_ax1 = ax1.get_xlim()
ax2.set_xlim(pC2MIP(xlim_ax1[0]), pC2MIP(xlim_ax1[1]))
ticks_ax2 = pC2MIP(np.array(ax1.get_xticks()))
ax2.set_xticks(ticks_ax2)
rounded_labels_ax2 = [round(tick) for tick in ticks_ax2]
ax2.set_xticklabels(rounded_labels_ax2)
ax2.set_xlabel('Number of MIPs')
ax2.spines['top'].set_color('crimson')
ax2.xaxis.label.set_color('crimson')
ax2.xaxis.set_label_coords(0.5, 1.07)
ax2.tick_params(axis = 'x', colors = 'crimson', direction = 'out', length = 5)
#ax2.set_xticks(range(int(pC2MIP(0)), int(pC2MIP(120)), 100))

# 3rd x-axis
ax3 = ax1.twiny()
ax3.set_xlim(pC2MeV(xlim_ax1[0]), pC2MeV(xlim_ax1[1]))
ticks_ax3 = pC2MeV(np.array(ax1.get_xticks()))
ax3.set_xticks(ticks_ax3)
ax3.set_xlabel('Energy [MeV]')
ax3.spines['top'].set_color('royalblue')
ax3.xaxis.label.set_color('royalblue')
ax3.xaxis.set_label_coords(0.5, 0.91)
ax3.xaxis.label.set_color('royalblue')
ax3.tick_params(axis = 'x', colors = 'royalblue', direction = 'in', length = 5, pad = -20 )

# 4th x-axis
ax4 = ax1.twiny()
ax4.set_xlim(pC2PE(xlim_ax1[0]), pC2PE(xlim_ax1[1]))
ticks_ax4 = pC2PE(np.array(ax1.get_xticks()))
ax4.set_xticks(ticks_ax4)
ax4.xaxis.set_label_coords(0.5, 0.07)
ax4.set_xlabel('Number of Photoelectrons')
ax4.spines['top'].set_position(('axes', 1))  # Position it on the primary x-axis
ax4.xaxis.set_ticks_position('bottom')
#ax4.xaxis.set_label_position('bottom')
ax4.spines['bottom'].set_position(('outward', 0))
ax4.spines['bottom'].set_color('black')
ax4.xaxis.label.set_color('darkmagenta')
ax4.tick_params(axis='x', colors='darkmagenta', direction='in', length = 5, pad = -15)

# Some modifications
ax1.spines["right"].set_visible(False)
ax1.spines["left"].set_visible(False)
ax1.spines["top"].set_visible(False)
ax2.spines["right"].set_visible(False)
ax2.spines["left"].set_visible(False)
ax2.spines["top"].set_visible(False)
ax3.spines["right"].set_visible(False)
ax3.spines["left"].set_visible(False)
ax3.spines["top"].set_visible(False)
ax4.spines["right"].set_visible(False)
ax4.spines["left"].set_visible(False)
ax4.spines["top"].set_visible(False)

plt.show() 
