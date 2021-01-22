# Part. 1

#=======================================

# Import module

#  csv -- fileIO operation

import csv
import collections
#=======================================


# Part. 2

#=======================================

# Read cwb weather data

cwb_filename = '106010006.csv'

data = []

header = []

with open(cwb_filename) as csvfile:

   mycsv = csv.DictReader(csvfile)

   header = mycsv.fieldnames

   for row in mycsv:

      data.append(row)

   

#=======================================


# Part. 3

#=======================================

# Analyze data depend on your group and store it to target_data like:

# Retrive all data points which station id is "C0X260" as a list.

target_data1 = list(filter(lambda item: item['WDSD'] != '-99.000', data))
target_data2 = list(filter(lambda item: item['WDSD'] != '-999.000', target_data1))
#print(target_data)

id =['C0A880','C0F9A0','C0G640','C0R190','C0X260']
d= collections.OrderedDict()
for i in id:
   #print(i)
   target_data3 = list(filter(lamba item: item['station_id'] == i, target_data2))
   #print(target_data2)
   min = float(target_data3[0]['WDSD'])
   max = float(target_data3[0]['WDSD'])
   for k in range(len(target_data3)):
      for j in range(len(target_data3)):
         if (float(target_data3[j]['WDSD']) < min):
            min = float(target_data3[j]['WDSD'])
         if (float(target_data3[j]['WDSD']) > max):
            max = float(target_data3[j]['WDSD'])
   max_range = max - min
   #print(max)
   #print(min)
   #print(max_range)
   d[i]=max_range
vd = collections.OrderedDict(sorted(d.items(),key=lambda t:t[1]))
print (vd)




# Retrive ten data points from the beginning.

#target_data = data[:20]


#=======================================


# Part. 4

#=======================================

# Print result

#print(target_data)

#========================================
