#! /usr/bin/python
# -*- coding: utf-8 -*-

import locale
import sys
import os
import os.path
sys.path.append("./lib")
import xlrd
import xlwt

# Use locale to convert number
locale.setlocale(locale.LC_ALL, 'en_US.UTF-8')
# For windows
# locale.setlocale(locale.LC_ALL, 'usa')

_ORGS_TO_WIRTE = [
        u'宝坻', u'北辰', u'滨海', u'大港', u'东丽', u'高新区', u'汉沽', u'和平',
        u'河北', u'河东', u'河西', u'红桥', u'蓟县', u'津南', u'静海', u'开发区',
        u'自贸区', u'南开', u'宁河', u'塘沽', u'武清', u'西青', u'营业部']

_ORG_IDX = 0
_VOLUME_IDX = 1
_CHANNEL_IDX = 2
_header_style = xlwt.Style.easyxf(u'font: name 楷体_GB2312, bold true, height 280;')
_data_style = xlwt.Style.easyxf(u'font: name 楷体_GB2312, bold false, height 280;')

class FoundProcessor(object):
    _ORG_TITLE = u'交易机构'
    _VOLUME_TITLE = u'交易金额'
    _INIT_TITLE = u'发起方'
    _CHANNEL_TITLE = u'交易渠道'

    def __init__(self, file_path):
        self.__file = file_path
        self.__book = xlrd.open_workbook(self.__file)
        self.__rows = []
        pass

    def process(self):
        sheet = self.__book.sheets()[0]
        self.__parse_header(sheet)
        self.__extract_data(sheet)
        pass

    def get_rows(self):
        return self.__rows

    def __parse_header(self, sheet):
        self.__row_no = 0
        # Skip empty line
        while self.__row_no < sheet.nrows:
            row = sheet.row_values(self.__row_no)
            find = False
            for title in row:
                if title == self._ORG_TITLE:
                    find = True
                    break;
                pass
            if find:
                break
            self.__row_no += 1
            pass
        self.__row_no += 1
        # Find which column we need
        idx = 0
        # __init_col_no may not exist, if it is not exist, 
        # ignore it
        self.__init_col_no = -1
        for title in row:
            if title == self._ORG_TITLE:
                self.__org_col_no = idx
            elif title == self._VOLUME_TITLE:
                self.__volume_col_no = idx
            elif title == self._INIT_TITLE:
                self.__init_col_no = idx
            elif title == self._CHANNEL_TITLE:
                self.__channel_col_no = idx
            idx += 1
        pass

    def __extract_data(self, sheet):
        for self.__row_no in range(self.__row_no, sheet.nrows):
            row = sheet.row_values(self.__row_no)
            # continus if initiator is 'TA'
            if self.__init_col_no != -1 and row[self.__init_col_no].find(u'TA') != -1:
                continue
            org = row[self.__org_col_no]
            sub = self.__get_sub_from_org(org)
            if type(row[self.__volume_col_no]) is float:
                volume = row[self.__volume_col_no] / 10000
            else:
                volume = locale.atof(row[self.__volume_col_no]) / 10000
            channel = row[self.__channel_col_no].partition(u':')[2]

            self.__rows.append([sub, volume, channel])
            pass
        pass

    def __get_sub_from_org(self, org):
        sub_branch_names = [
                u'宝坻', u'北辰', u'滨海', u'大港', u'东丽', u'高新区', u'汉沽', u'和平', u'河北', 
                u'河东', u'河西', u'红桥', u'蓟县', u'津南', u'静海', u'开发区', u'自贸区', u'南开',
                u'宁河', u'塘沽', u'武清', u'西青', u'营业部', u'梅江', 
                u'天房美域分理处', u'天合家园分理处']
        pos = -1
        for name in sub_branch_names:
            new_pos = org.find(name);
            if new_pos != -1 and (new_pos < pos or pos == -1):
                sub = name
                pos = new_pos
            pass
        if sub == u'梅江':
            sub = u'河西'
            pass
        elif sub == u'天房美域分理处':
            sub = u'西青'
            pass
        elif sub == u'天合家园分理处':
            sub = u'东丽'
            pass
        return sub

    def __debug_agg_table(self):
        for row in self.__rows:
            print ", ".join(map(lambda x:unicode(x), row))
    pass

class FoundDirProcessor(object):
    global _ORGS_TO_WIRTE
    global _ORG_IDX
    global _VOLUME_IDX
    global _CHANNEL_IDX
    global _header_style
    global _data_style
    def __init__(self, input_dir, book):
        self.__input_dir = input_dir
        self.__book = book
        pass

    def process(self):
        files = os.listdir(self.__input_dir)
        for fname in files:
            if not fname.endswith(".xls"):
                continue
            if fname.startswith('.'):
                continue
            self.__process_one_file(fname)
            pass
        pass
    
    def __process_one_file(self, fname):
        fpath = os.path.join(self.__input_dir, fname)
        file_handler = FoundProcessor(fpath)
        file_handler.process()
        tbl_by_org = {}
        tbl_by_channel = {}
        self.__agg_rows(file_handler.get_rows(), tbl_by_org, tbl_by_channel)
        sheet_name = os.path.splitext(fname)[0]

        # write organization
        sheet = self.__book.add_sheet(sheet_name)
        self.__write_org_tbl(tbl_by_org, sheet)
        # write channel
        sheet = self.__book.add_sheet(sheet_name + u'_channel')
        self.__write_channel_tbl(tbl_by_channel, sheet)
        pass

    def __agg_rows(self, rows, tbl_by_org, tbl_by_channel):
        for row in rows:
            org = row[_ORG_IDX]
            channel = row[_CHANNEL_IDX]
            volume = row[_VOLUME_IDX]

            # Aggregate org
            if tbl_by_org.has_key(org):
                tbl_by_org[org][0] += 1
                tbl_by_org[org][1] += volume
            else:
                tbl_by_org[org] = [1, volume]

            # Aggregate channel
            if tbl_by_channel.has_key(channel):
                tbl_by_channel[channel][0] += 1
                tbl_by_channel[channel][1] += volume
            else:
                tbl_by_channel[channel] = [1, volume]
        pass

    def __write_org_tbl(self, tbl, sheet):
        # Wirte header first
        sheet.write(0, 0, u'机构名称', _header_style)
        sheet.write(0, 1, u'笔数', _header_style)
        sheet.write(0, 2, u'金额', _header_style)

        row_no = 1
        trade_sum = 0
        volume_sum = 0.00
        for org in _ORGS_TO_WIRTE:
            if tbl.has_key(org):
                num_trades = tbl[org][0]
                volume = tbl[org][1]
            else:
                num_trades = 0
                volume = 0.00

            trade_sum += num_trades
            volume_sum += volume
            sheet.write(row_no, 0, org, _data_style)
            sheet.write(row_no, 1, num_trades, _data_style)
            sheet.write(row_no, 2, volume, _data_style)
            row_no += 1
            pass
        sheet.write(row_no, 0, u'合计', _data_style)
        sheet.write(row_no, 1, trade_sum, _data_style)
        sheet.write(row_no, 2, volume_sum, _data_style)
        pass

    def __write_channel_tbl(self, tbl, sheet):
        # Wirte header first
        sheet.write(0, 0, u'交易渠道', _header_style)
        sheet.write(0, 1, u'笔数', _header_style)
        sheet.write(0, 2, u'金额', _header_style)

        row_no = 1
        trade_sum = 0
        volume_sum = 0.00
        for channel in tbl.iterkeys():
            num_trades = tbl[channel][0]
            volume = tbl[channel][1]

            trade_sum += num_trades
            volume_sum += volume
            sheet.write(row_no, 0, channel, _data_style)
            sheet.write(row_no, 1, num_trades, _data_style)
            sheet.write(row_no, 2, volume, _data_style)
            row_no += 1
            pass
        sheet.write(row_no, 0, u'合计', _data_style)
        sheet.write(row_no, 1, trade_sum, _data_style)
        sheet.write(row_no, 2, volume_sum, _data_style)
        pass

class PayDirProcessor(object):
    global _ORGS_TO_WIRTE
    global _ORG_IDX
    global _VOLUME_IDX
    global _CHANNEL_IDX
    global _header_style
    global _data_style

    def __init__(self, input_dir, book):
        self.__input_dir = input_dir
        self.__book = book
        self.__agg_tbl = {}
        for org in _ORGS_TO_WIRTE:
            self.__agg_tbl[org] = 0.00
        pass

    def process(self):
        files = os.listdir(self.__input_dir)
        # Process files into __agg_tbl
        for fname in files:
            if not fname.endswith(".xls"):
                continue
            if fname.startswith('.'):
                continue
            self.__process_one_file(fname)
            pass

        # write result to sheet
        sheet_name = u'兑付'
        sheet = self.__book.add_sheet(sheet_name)
        self.__write_to_sheet(sheet)

        pass

    def __process_one_file(self, fname):
        fpath = os.path.join(self.__input_dir, fname)
        file_handler = FoundProcessor(fpath)
        file_handler.process()
        for row in file_handler.get_rows():
            org = row[_ORG_IDX]
            volume = row[_VOLUME_IDX]
            self.__agg_tbl[org] += volume
        pass

    def __write_to_sheet(self, sheet):
        # Wirte header first
        sheet.write(0, 0, u'结构名称', _header_style)
        sheet.write(0, 1, u'金额', _header_style)

        # Write data
        row_no = 1
        trade_sum = 0
        volume_sum = 0.00
        for org in _ORGS_TO_WIRTE:
            volume = self.__agg_tbl[org]

            volume_sum += volume
            sheet.write(row_no, 0, org, _data_style)
            sheet.write(row_no, 1, volume, _data_style)
            row_no += 1
            pass

        # Write sum
        sheet.write(row_no, 0, u'合计', _data_style)
        sheet.write(row_no, 1, volume_sum, _data_style)
        pass

if __name__ == '__main__':
    book = xlwt.Workbook()
    processor = FoundDirProcessor("./new_found", book)
    processor.process()
    processor = PayDirProcessor("./兑付数据", book)
    processor.process()
    book.save('./result.xls')
    pass






















