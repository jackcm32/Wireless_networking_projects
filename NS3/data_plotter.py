
import json
import pandas as pd
import numpy as np
import itertools
import sys
import math
from matplotlib import pyplot as plt

def walls_vs_pktloss():

	# Lenth of lists to be plotted
	list_len = int(sys.argv[2])
	list1 = sys.argv[3:list_len+3]
	list2 = sys.argv[list_len+3:(2*list_len)+3]

	plt.scatter(list1, list2, c='r', alpha=0.5)
	
	plt.title('Packet loss vs. Rooms')
	plt.ylabel('Number of lost packets')
	plt.xlabel('Number of Walls obstructing path')

	plt.show()

def walls_vs_ave_pktloss():

	# Lenth of lists to be plotted
	list_len = int(sys.argv[2])
	list1 = sys.argv[3:list_len+3]
	list2 = sys.argv[list_len+3:(2*list_len)+3]

	pkt_loss_df = pd.DataFrame([list2,list1])
	pkt_loss_df = pkt_loss_df.T.convert_objects(convert_numeric=True)

	ave_pkt_loss_df = pd.DataFrame()
	ave_pkt_loss_df['mean'] = pkt_loss_df.groupby([1])[0].mean()
	ave_pkt_loss_df['std dev'] = pkt_loss_df.groupby([1])[0].std(ddof=1)

	ave_pkt_loss_df['error bar vals'] = ave_pkt_loss_df['std dev']/math.sqrt(list_len)

	error_config = {'ecolor': '0.3'}

	plt.bar( ave_pkt_loss_df.index, ave_pkt_loss_df['mean'], color='r',yerr=ave_pkt_loss_df['error bar vals'], align='center', error_kw=error_config)
	
	plt.title('Average Packet loss vs. Rooms')
	plt.ylabel('Average number of lost packets')
	plt.xlabel('Number of Walls obstructing path')

	plt.tight_layout()
	plt.show()

def square(list):
    return [i ** 2 for i in list]

def size_vs_ave_pktloss():

	# Lenth of lists to be plotted
	list_len = int(sys.argv[2])
	list1 = sys.argv[3:list_len+3]
	list2 = sys.argv[list_len+3:(2*list_len)+3]

	pkt_loss_df = pd.DataFrame([list2,list1])
	pkt_loss_df = pkt_loss_df.T.convert_objects(convert_numeric=True)

	ave_pkt_loss_df = pd.DataFrame()
	ave_pkt_loss_df['mean'] = pkt_loss_df.groupby([1])[0].mean()
	ave_pkt_loss_df['std dev'] = pkt_loss_df.groupby([1])[0].std(ddof=1)

	ave_pkt_loss_df['error bar vals'] = ave_pkt_loss_df['std dev']/math.sqrt(list_len)

	del ave_pkt_loss_df.index.name

	error_config = {'ecolor': '0.3'}

	plt.bar( ave_pkt_loss_df.index, ave_pkt_loss_df['mean'], color = 'r', yerr=ave_pkt_loss_df['error bar vals'].values, align='center', error_kw=error_config)
	
	area_vals = ( 8 + ave_pkt_loss_df.index*2)
	plt.xticks(ave_pkt_loss_df.index, area_vals )  


	plt.ylim([2000,2200])
	plt.title('Average Packet loss vs. Room size')
	plt.ylabel('Average number of lost packets')
	plt.xlabel('Room aera (m^2)')

	plt.tight_layout()
	plt.show()


def main():



	if(str(sys.argv[1]) == 'wall'):
		walls_vs_pktloss()
		walls_vs_ave_pktloss()
	elif(str(sys.argv[1]) == 'room'):
		size_vs_ave_pktloss()



if __name__ == "__main__":
	main()



