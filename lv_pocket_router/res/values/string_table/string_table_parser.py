# String translation table parse
import csv
import os

row_header = 2
col_header = 1

csv_file = os.getcwd() + '/string_table.csv'
string_value_h = os.getcwd() + '/../string_value.h'
strings_h = os.getcwd() + '/../string_table.h'

if (not os.path.isfile(csv_file)):
    print('Error!!!\nfile not found ' + csv_file)
    exit()

csv_col_count = 0

# Create string_value.h
with open(string_value_h, "w") as header_output:
    # file header
    header_output.write("#ifndef _MYSTRING_H_\n#define _MYSTRING_H_\n\n")

    # enum LOCALE
    i = 0
    header_output.write("enum LOCALE {\n")
    with open(csv_file) as csvfile:
        readCSV = csv.reader(csvfile, delimiter='\t')
        for row in readCSV:
            i += 1

            if i == col_header:
                locale = row[0].replace('STRING ID,DESCRIPTION,', '    ')
                csv_col_count = locale.count(',') + 1
                locale = locale.replace(',', " = 0,", 1)
                header_output.write(locale.replace(',', ',\n    '))
                header_output.write(",\n    LOCALE_COUNT\n};\n\n")
                continue

            # enum RES_IDS
            if i == row_header:
                header_output.write("enum RES_IDS {\n")
                header_output.write("    " + row[0].split(',')[0] + " = 0")
                continue
            header_output.write(",\n    " + row[0].split(',')[0])

    csvfile.close()

    # file footer
    header_output.write("\n};\n\nextern const char* get_string(int res_id);")
    header_output.write("\nextern const char* get_string_locale(int res_id, int locale);")
    header_output.write("\nextern void set_device_locale(int locale);")
    header_output.write("\nextern void config_locale();")
    header_output.write("\nint get_device_locale();")
    header_output.write("\n\n#endif // _MYSTRING_H_")

    header_output.close()
    csvfile.close()


# Create string_table.h
with open(strings_h, "w") as strings_output:
    # file header
    strings_output.write("#ifndef LV_POCKET_ROUTER_RES_VALUES_STRINGS_H_\n#define LV_POCKET_ROUTER_RES_VALUES_STRINGS_H_\n\n")

    i = 0
    strings_output.write("const char* string_table[][" + str(csv_col_count) + "] = {\n")

    with open(csv_file) as string_csv:
        readCSV = csv.reader(string_csv, delimiter=',', quotechar='"')
        for row in readCSV:
            i += 1

            # Skip header
            if i <= col_header:
                continue

            translation = "    {"
            for locale_count in range(2, csv_col_count + 2):
                translation = translation + "\"" + row[locale_count] + "\", "
            translation = translation[:-2] + '},\n'
            strings_output.write(translation)
        strings_output.write('};')

    # file footer
    strings_output.write("\n\n#endif /* LV_POCKET_ROUTER_RES_VALUES_STRINGS_H_ */")

    string_csv.close()
    strings_output.close()

print 'Done!!!\n' + os.path.abspath(string_value_h) + '\n' + os.path.abspath(strings_h)
