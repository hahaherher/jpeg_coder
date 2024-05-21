#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <map>
#include <bitset>

#include "dct2.h"

using namespace std;

struct RGB_Point {
    float R;
    float G;
    float B;
    RGB_Point(float r, float g, float b) { R = r, G = g, B = b; }
};

struct YCbCr_Point {
    float Y;
    float Cb;
    float Cr;
    YCbCr_Point(float y, float cb, float cr) { Y = y, Cb = cb, Cr = cr; }
};


YCbCr_Point toYCbCr(RGB_Point p){
    float Y = 0.299 * p.R + 0.587 * p.G + 0.114 * p.B;
    float Cb = -0.16875 * p.R - 0.33126 * p.G + 0.5 * p.B;
    float Cr = 0.5 * p.R - 0.41869 * p.G - 0.08131 * p.B;
    return YCbCr_Point(Y, Cb, Cr);
}


RGB_Point toRGB(YCbCr_Point p){
    float R = p.Y + 1.402 * p.Cr;
    float G = p.Y - 0.34413 * p.Cb - 0.71414 * p.Cr;
    float B = p.Y + 1.772 * p.Cb;
    return RGB_Point(R, G, B);
}



float** create_2D_array(int n) {
    // 二维 float array
    float** array_2D = new float* [n]; // 创建指向 float 指针的指针
    for (int i = 0; i < n; ++i) {
        array_2D[i] = new float[n]; // 为每行创建 float 数组
    }

    return array_2D;
}


float*** create_3D_array(int n) {
    // 二维 float array
    float*** array_3D = new float** [3]; // 创建指向 float 指针的指针
    for (int i = 0; i < 3; ++i) {
        array_3D[i] = create_2D_array(n);
    }
    return array_3D;
}


float*** toYCbCr_img(float*** RGB_img) {
    float*** YCbCr_img = create_3D_array(512);;
    // convert RGB to YCbCr
    for (int i = 0; i < 512; ++i) {
        for (int j = 0; j < 512; ++j) {
            for (int c = 0; c < 3; c++) {
                float R = RGB_img[0][i][j];
                float G = RGB_img[1][i][j];
                float B = RGB_img[2][i][j];
                YCbCr_Point p = toYCbCr(RGB_Point(R, G, B));
                YCbCr_img[0][i][j] = p.Y;
                YCbCr_img[1][i][j] = p.Cb;
                YCbCr_img[2][i][j] = p.Cr;
            }
        }
    }
    return YCbCr_img;
}


float*** toRGB_img(float*** YCbCr_img) {
    float*** RGB_img = create_3D_array(512);
    // convert RGB to YCbCr
    for (int i = 0; i < 512; ++i) {
        for (int j = 0; j < 512; ++j) {
            for (int c = 0; c < 3; c++) {
                float Y = YCbCr_img[0][i][j];
                float Cb = YCbCr_img[1][i][j];
                float Cr = YCbCr_img[2][i][j];
                RGB_Point p = toRGB(YCbCr_Point(Y, Cb, Cr));
                RGB_img[0][i][j] = p.R;
                RGB_img[1][i][j] = p.G;
                RGB_img[2][i][j] = p.B;
            }
        }
    }
    return RGB_img;
}

float*** read_color_raw_img(string file_name) {
    ifstream rawFile(file_name, ios::in | ios::binary);

    // 獲取檔案大小
    rawFile.seekg(0, ios::end);
    streampos fileSize = rawFile.tellg();
    rawFile.seekg(0, ios::beg);

    // 讀取所有數據到vector
    vector<unsigned char> temp_vec(fileSize);
    rawFile.read(reinterpret_cast<char*>(temp_vec.data()), fileSize);


    // 3维 float array
    int n = sqrt(int(fileSize/3));
    float*** original_img = create_3D_array(n);

    // 讀取 vector 數據到 float array
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            for (int c = 0; c < 3; c++) {
                original_img[c][i][j] = temp_vec[i * n*3 + j * 3 + c];
            }
        }
    }

    cout << "已成功讀取 " << int(fileSize) << " 個字節的數據" << endl << endl;

    // 關閉檔案
    rawFile.close();

    return original_img;
}


void subsampling(float*** YCbCr_img, string subsampling_mode) {
    int x_pos = 0, y_pos = 0;
    int sub_num = subsampling_mode[subsampling_mode.length() - 1] - 48;
    while (y_pos < 512) {
        // scan 4*4 block from 512*512 image
        for (int i = 0; i < 4; i++) {
            for (int c = 1; c < 3; c++) {
                float up_left = YCbCr_img[c][x_pos + i][y_pos];
                YCbCr_img[c][x_pos + i][y_pos + 1] = up_left;
                if (sub_num == 1) {
                    YCbCr_img[c][x_pos + i][y_pos + 2] = up_left;
                    YCbCr_img[c][x_pos + i][y_pos + 3] = up_left;
                }
                else{
                    YCbCr_img[c][x_pos + i][y_pos + 3] = YCbCr_img[c][x_pos + i][y_pos + 2];
                }
            }
        }
        // move to next 4*4 position
        x_pos += 4;
        if (x_pos == 512) {
            y_pos += 4;
            x_pos = 0;
        }
    }
}

float** read_raw_img(string file_name) {
    ifstream rawFile(file_name, ios::in | ios::binary);

    // 獲取檔案大小
    rawFile.seekg(0, ios::end);
    streampos fileSize = rawFile.tellg();
    rawFile.seekg(0, ios::beg);

    // 讀取所有數據到vector
    vector<unsigned char> temp_vec(fileSize);
    rawFile.read(reinterpret_cast<char*>(temp_vec.data()), fileSize);


    // 二维 float array
    int n = sqrt(int(fileSize));
    float** original_img = create_2D_array(n);
    //float** original_img = new float* [n]; // 创建指向 float 指针的指针

    //for (int i = 0; i < n; ++i) {
    //    original_img[i] = new float[n]; // 为每行创建 float 数组
    //}

    // 讀取 vector 數據到 float array
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            original_img[i][j] = temp_vec[i * n + j];
        }
    }

    cout << "已成功讀取 " << int(fileSize) << " 個字節的數據" << endl << endl;

    // 關閉檔案
    rawFile.close();

    return original_img;
}


// 函數用於解析 CSV 檔案中的一行數據並返回一個結構
map<vector<int>, string> parseCSV(string filename) {
    // 打開 CSV 檔案
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "無法打開檔案" << endl;
    }

    // 存儲所有行數據的向量
    map<vector<int>, string> dataList;

    // 逐行讀取檔案內容
    string line;
    while (getline(file, line)) {
        // 解析每一行並將數據存儲到 dataList 向量中
        vector<int> ac_indices;
        stringstream ss(line);
        string token;

        // 逐一讀取每個逗號分隔的欄位
        // run
        getline(ss, token, ',');
        ac_indices.push_back(stoi(token));
        // category
        getline(ss, token, ',');
        ac_indices.push_back(stoi(token));
        // codeword
        getline(ss, token, ',');
        dataList[ac_indices] = token;
    }
    file.close();

    return dataList;
}


int QUAN_MATRIX[8][8] = {
        {16, 11, 10, 16, 24, 40, 51, 61},
        {12, 12, 14, 19, 26, 58, 60, 55},
        {14, 13, 16, 24, 40, 57, 69, 56},
        {14, 17, 22, 29, 51, 87, 80, 62},
        {18, 22, 37, 56, 68, 109, 103, 77},
        {24, 35, 55, 64, 81, 104, 113, 92},
        {49, 64, 78, 87, 103, 121, 120, 101},
        {72, 92, 95, 98, 112, 100, 103, 99},
};

int QUAN_CHROM_MATRIX[8][8] = {
        {17, 18, 24, 47, 99, 99, 99, 99},
        {18, 21, 26, 66, 99, 99, 99, 99},
        {24, 26, 56, 99, 99, 99, 99, 99},
        {47, 66, 99, 99, 99, 99, 99, 99},
        {99, 99, 99, 99, 99, 99, 99, 99},
        {99, 99, 99, 99, 99, 99, 99, 99},
        {99, 99, 99, 99, 99, 99, 99, 99},
        {99, 99, 99, 99, 99, 99, 99, 99},
};

void quantize(float** x, int QF, int c) {

    float factor;
    if (QF < 50) {
        factor = 5000 / QF;
    }
    else {
        factor = 200 - 2 * QF;
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            float qij;
            if (c == 0) {
                qij = QUAN_MATRIX[i][j] * factor / 100;
            }
            else {
                qij = QUAN_CHROM_MATRIX[i][j] * factor / 100;
            }
            x[i][j] = round(x[i][j] / qij);
            //x[i][j] = round(x[i][j]);
        }
    }
}


void invert_quantize(float** x, int QF, int c) {
    float factor;
    if (QF < 50) {
        factor = 5000 / QF;
    }
    else {
        factor = 200 - 2 * QF;
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            float qij;
            if (c == 0) {
                qij = QUAN_MATRIX[i][j] * factor / 100;
            }
            else {
                qij = QUAN_CHROM_MATRIX[i][j] * factor / 100;
            }
            x[i][j] *= qij;
            x[i][j] = round(x[i][j]);
        }
    }
}


struct SnakeBody {
    int zeros;
    float value;
    SnakeBody(int z, float v) { zeros = z; value = v; };
    bool operator==(const SnakeBody& s2) const{
        return zeros == s2.zeros && value == s2.value;
    }
};

vector<SnakeBody> AC_run_length(float** block) {
    vector<SnakeBody> snake_vec;
    int sign = -1;
    int i = 1, j = 0;
    int time_increse = 1;
    int zeros = 0;
    for (int times = 1; times <= 7 && times >= 0; times += time_increse, sign *= -1) {
        for (int time_count = times; time_count >= 0; time_count--, i += sign, j -= sign) {
            if (block[i][j] == 0) {
                zeros++;
            }
            else {
                if (zeros > 15) {
                    for (int fifth_id = 0; fifth_id < floor(zeros / 15); fifth_id++) {
                        snake_vec.push_back(SnakeBody(15, 0));
                        zeros -= 15;
                        if (zeros <= 15) {
                            snake_vec.push_back(SnakeBody(zeros, block[i][j]));
                        }
                    }
                }
                else {
                    snake_vec.push_back(SnakeBody(zeros, block[i][j]));
                }
                zeros = 0;
            }
        }
        i -= sign;
        j += sign;

        if (times == 7) {
            time_increse = -1;
        }
        if (sign == 1 && time_increse == 1 || sign == -1 && time_increse == -1) {
            i += 1;
        }
        else if (sign == -1 && time_increse == 1 || sign == 1 && time_increse == -1) {
            j += 1;
        }

    }
    /*while (snake_vec.size() > 0 ) {
        if (snake_vec[snake_vec.size() - 1].value == 0 && snake_vec[snake_vec.size() - 1].zeros == 15) {
            snake_vec.pop_back();
        }
        else {
            break;
        }
    }*/
    
    snake_vec.push_back(SnakeBody(-1, 0));

    return snake_vec;
}


// 將整數映射到對應的二進制字符串
string intToBinaryString(int num) {
    if (num == 1)
        return "1";
    else if (num == -1)
        return "0";

    string binaryString;
    while (num != 0) {
        if (num % 2 == 0 && num > 0 || num % 2 != 0 && num < 0)
            binaryString = "0" + binaryString;
        else
            binaryString = "1" + binaryString;
        num /= 2;
    }
    return binaryString;
}

// 將二進制字符串轉換回整數
int binaryStringToInt(string binaryString) {
    int num = 0;
    if (binaryString[0] == '0') {
        string positiveBinary;
        for (int i = 0; i < binaryString.size(); ++i) {
            positiveBinary += (binaryString[i] == '0') ? '1' : '0';
        }
        for (int i = 0; i < positiveBinary.size(); ++i) {
            if (positiveBinary[i] == '1') {
                num += 1 << (positiveBinary.size() - 1 - i);
            }
        }
        num *= -1;
    }
    else {
        // 正數的處理
        for (int i = 0; i < binaryString.size(); ++i) {
            if (binaryString[i] == '1') {
                num += 1 << (binaryString.size() - 1 - i);
            }
        }
    }

    return num;
}

map<vector<int>, string> AC_table = parseCSV("AC_Luminance.csv");

string AC_encode(vector<SnakeBody> snake_vec) {
    //map<int, string> conversionMap;
    //// 建立整數到二進制字符串的映射
    //for (int i = -60; i <= 60; ++i) {
    //    conversionMap[i] = intToBinaryString(i);
    //}

    //// 輸出映射結果
    //for (const auto& pair : conversionMap) {
    //    cout << pair.first << " -> " << pair.second << endl;
    //}

    string bitstring = "";

    for (SnakeBody s : snake_vec) {
        string codeword;
        int category = floor(log2(abs(s.value))) + 1;
        if (s.zeros == -1) {
            codeword = "1010";
        }
        else if (s.zeros == 15 && s.value == 0) {
            codeword = AC_table[{15, 0}];
        }
        else {
            string coefficient_codeword = intToBinaryString(s.value);
            vector<int> ac_indices = { s.zeros , category };
            string temp_str = AC_table[ac_indices];
            codeword = AC_table[ac_indices] + coefficient_codeword;
        }
        bitstring += codeword;
    }

    return bitstring;
}


vector<string> DC_table = {
    // DC Luminance table
    "00", "010", "011", "100", "101", "110",
    "1110", "11110", "111110", "1111110", "11111110", "111111110"
};

map<int, string> diff_table;
map<string, int> invert_diff_table;
map<string, vector<int>> invert_AC_table;
map<string, int> invert_DC_table;

void get_invert_tables() {
    // build a new table equals to map<code, diff>
    for (const auto& pair : AC_table) {
        invert_AC_table[pair.second] = pair.first;
    }
    // build a new table equals to map<code, diff>
    for (int i = 0; i < DC_table.size(); i++) {
        invert_DC_table[DC_table[i]] = i;
    }
}

string diff_search(int diff_DC) {
    string diff_codeword;

    auto it = diff_table.find(diff_DC);
    if (it != diff_table.end()) {
        diff_codeword = diff_table[diff_DC];
    }
    else {
        diff_codeword = intToBinaryString(diff_DC);
        diff_table[diff_DC] = diff_codeword;
        invert_diff_table[diff_codeword] = diff_DC;
    }
    return diff_codeword;
}

int diff_search(string diff_codeword) {
    int diff_DC;
    auto it = invert_diff_table.find(diff_codeword);
    if (it != invert_diff_table.end()) {
        diff_DC = invert_diff_table[diff_codeword];

    }
    else {
        diff_DC = binaryStringToInt(diff_codeword);
        invert_diff_table[diff_codeword] = diff_DC;
        diff_table[diff_DC] = diff_codeword;
    }
    return diff_DC;
}


string DC_encode(int diff_DC) {
    string dc_codewords;
    string diff_codeword;// = intToBinaryString(diff_DC);

    diff_codeword = diff_search(diff_DC);
    int category = (diff_DC == 0) ? 0 : floor(log2(abs(diff_DC))) + 1;

    dc_codewords = DC_table[category] + diff_codeword;

    return dc_codewords;
}


// calculate PSNR, QF=90, 80, 50, 20, 10 and 5
double calculatePSNR(float** original_img, float** processed_img, int image_width) {
    int max = 0;
    int sum = 0;
    for (int i = 0; i < image_width; i++) {
        for (int j = 0; j < image_width; j++) {
            if (original_img[i][j] > max) max = original_img[i][j];
            int diff = (original_img[i][j] - processed_img[i][j]);
            sum += diff * diff;
        }
    }
    double MSE = (double)sum / (image_width * image_width);
    double PSNR = 10.0f * log10((double)max * max / MSE);

    return PSNR;
}


double calculatePSNR(float*** original_img, float*** processed_img, int image_width) {
    int max = 0;
    double sum = 0;
    for (int c = 0; c < 3; c++) {
        for (int i = 0; i < image_width; i++) {
            for (int j = 0; j < image_width; j++) {
                if (original_img[c][i][j] > max) max = original_img[c][i][j];
                int diff = (original_img[c][i][j] - processed_img[c][i][j]);
                sum += diff * diff;
            }
        }
    }
    double MSE = (double)sum / (image_width * image_width * 3);
    //cout << MSE<<" "<<max * max / MSE << endl;
    double PSNR = 10.0f * log10((double)max * max / MSE);

    return PSNR;
}


void write_jpg(string filename, string bitstream) {
    // write bitstream as bitset type
    size_t strlen = bitstream.length();
    //cout << strlen << endl << endl;

    ofstream outputFile(filename, ios::binary);
    if (outputFile.is_open()) {
        //write bitstream length
        outputFile.write(reinterpret_cast<const char*>(&strlen), sizeof(size_t));

        // create a buffer to save bit
        bitset<32> buffer;
        size_t bufferIndex = 0; //size_t = unsigned int32 = 32 bits

        // convert to binary
        for (char c : bitstream) {
            if (c == '1') {
                buffer.set(bufferIndex);
            }
            ++bufferIndex;
            if (bufferIndex == 32) { // write to file when buffer is full
                outputFile.write(reinterpret_cast<const char*>(&buffer), sizeof(buffer));
                buffer.reset();
                bufferIndex = 0;
            }
        }
        // write to file if there's still data in buffer
        if (bufferIndex > 0) {
            outputFile.write(reinterpret_cast<const char*>(&buffer), sizeof(buffer));
        }
        outputFile.close();
        //cout << "Binary data has been written to file.\n" << endl;
    }
    else {
        cerr << "Unable to open file." << endl;
    }
}


string read_jpg(string filename) {
    //read binary file
    ifstream jpgFile(filename, ios::in | ios::binary);

    string bitstream = "";
    map<unsigned char, vector<bool>> huffmanTable;
    size_t mapSize;


    // 逐字节读取文件内容
    if (jpgFile.is_open()) {
        bitset<32> bits;  //62KB
        size_t lastNonZero;
        jpgFile.read(reinterpret_cast<char*>(&lastNonZero), sizeof(size_t));
        while (jpgFile.read(reinterpret_cast<char*>(&bits), sizeof(bits))) {
            // convert to char
            for (size_t i = 0; i < 32; ++i) {
                if (bits.test(i)) {
                    bitstream += '1';
                }
                else {
                    bitstream += '0';
                }
            }
        }
        jpgFile.close();

        // 去除末尾的多余零
        if (lastNonZero != string::npos) {
            bitstream = bitstream.substr(0, lastNonZero);
        }
        else {
            bitstream.clear(); // 如果全是0，则清空字符串
        }
        //cout << "Read " << bitstream.length() << " bits." << endl;

        jpgFile.close();
        return bitstream;
    }
    else {
        cerr << "Unable to open file." << endl;
    }
}

string str_pop(string str, int pop_num) {
    str.erase(0, pop_num);
    //while (pop_num > 0) {
    //    //str.pop_back();
    //    
    //    pop_num--;
    //}
    return str;
}

string compressed_bitstream;

int dc_decode(int last_DC) {
    // DC decode
    int bit_num = 4;
    int category;
    string DC_bits = compressed_bitstream.substr(0, bit_num);
    // < 1110
    if (DC_bits < DC_table[6]) {
        DC_bits = compressed_bitstream.substr(0, --bit_num);

        if (invert_DC_table.find(DC_bits) != invert_DC_table.end()) {
            category = invert_DC_table[DC_bits];
        }
        else {
            bit_num--;
            category = 0;
        }
    }
    // > 1110
    else {
        while (DC_bits[bit_num - 1] != '0') {
            DC_bits = compressed_bitstream.substr(0, ++bit_num);
        }
        category = DC_bits.length() + 2;
    }
    compressed_bitstream = str_pop(compressed_bitstream, bit_num);

    string diff_DC_codeword = compressed_bitstream.substr(0, category);
    int diff_DC = diff_search(diff_DC_codeword);
    compressed_bitstream = str_pop(compressed_bitstream, category);

    int DC = last_DC + diff_DC;
    return DC;
}


vector<SnakeBody> ac_decode() {
    // AC decode
    vector<SnakeBody> ac_snake;
    int bit_num, category;
    while (compressed_bitstream.length() > 0)
    {
        string ac_codeword = compressed_bitstream.substr(0, 4);

        if (ac_codeword.compare("1010") == 0) {
            compressed_bitstream = str_pop(compressed_bitstream, 4);
            ac_snake.push_back(SnakeBody(-1, 0));
            break;
        }
        bit_num = 2;
        ac_codeword = compressed_bitstream.substr(0, bit_num);
        while (invert_AC_table.find(ac_codeword) == invert_AC_table.end()) {
            ac_codeword = compressed_bitstream.substr(0, ++bit_num);
        }
        compressed_bitstream = str_pop(compressed_bitstream, bit_num);

        vector<int> indices = invert_AC_table[ac_codeword];
        int run = indices[0];
        category = indices[1];
        string diff_codeword = compressed_bitstream.substr(0, category);
        compressed_bitstream = str_pop(compressed_bitstream, category);

        int diff_AC = (category==0) ? 0: diff_search(diff_codeword);
        ac_snake.push_back(SnakeBody(run, diff_AC));
    }
    return ac_snake;
}

float** invert_AC_run_length(int DC, vector<SnakeBody> ac_snake) {
    int n = 8;
    float** block = create_2D_array(n);

    int sign = -1;
    int i = 1, j = 0;
    int time_increse = 1;
    int snake_id = 0;
    int zeros;
    if (ac_snake.size() == 0) {
        zeros = n * n - 1;
    }
    else {
        zeros = ac_snake[snake_id].zeros;
    } 

    block[0][0] = DC;
    for (int times = 1; times <= 7 && times >= 0; times += time_increse, sign *= -1) {
        for (int time_count = times; time_count >= 0; time_count--, i += sign, j -= sign) {
            if (zeros > 0) {
                zeros--;
                block[i][j] = 0;
            }
            else {
                int value;
                if (ac_snake.size() == 0) value = 0; 
                else value = ac_snake[snake_id].value;
                
                // <15,0>
                if ((value == 0 && zeros == 0)){
                    snake_id++;
                    zeros = ac_snake[snake_id].zeros;
                    zeros -= 1;
                    //value = ac_snake[snake_id].value;
                }
                block[i][j] = value;
                
                // not EOF, snake_id++
                if (value != 0 ){//} || value == 0 && zeros == 0) {
                    snake_id++;
                    zeros = ac_snake[snake_id].zeros;
                }
                // EOF, value = 0 -> push 0 until the end
            }
        }
        i -= sign;
        j += sign;

        if (times == 7) {
            time_increse = -1;
        }
        if (sign == 1 && time_increse == 1 || sign == -1 && time_increse == -1) {
            i += 1;
        }
        else if (sign == -1 && time_increse == 1 || sign == 1 && time_increse == -1) {
            j += 1;
        }

    }

    return block;
}


char* str2chararr(string str_name) {
    char* char_name = new char[str_name.length() + 1];
    errno_t errcode2 = strcpy_s(char_name, str_name.length() + 1, str_name.c_str());
    return char_name;
}


void write_gray_img(float** gray_img, string decode_raw_name, int image_width) {
    ofstream outputFile(decode_raw_name, ios::binary);
    if (outputFile.is_open()) {
        for (int i = 0; i < image_width; ++i) {
            for (int j = 0; j < image_width; ++j) {
                unsigned char value = gray_img[i][j];
                outputFile.write(reinterpret_cast<const char*>(&value), sizeof(value));
            }
        }
        outputFile.close();
    }
    else {
        cerr << "Unable to open file." << endl;
    }
}

void write_color_img(float*** color_img, string decode_raw_name, int image_width) {
    ofstream outputFile(decode_raw_name, ios::binary);
    if (outputFile.is_open()) {
        for (int i = 0; i < image_width; ++i) {
            for (int j = 0; j < image_width; ++j) {
                for (int c : {0, 1, 2}) {
                    unsigned char value = color_img[c][i][j];
                    outputFile.write(reinterpret_cast<const char*>(&value), sizeof(value));
                }
            }
        }
        outputFile.close();
    }
    else {
        cerr << "Unable to open file." << endl;
    }
}

int main() {
    //float** gray_img = read_raw_img("./Test Images/GrayImages/Lena.raw");
    //write_gray_img(gray_img, "Lena_test.raw", 512);
    //cout << "Lena_test.raw" << endl;
    /*int temp_block[8][8] = {
    {226, 0, 254, 0, 0, 0, 0, 0},
    {255, 255, 0, 0, 0, 0, 0, 0},
    {255, 0, 0, 0, 0, 0, 0, 0},
    {1, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0}};*/

    /*int temp_block[8][8] = {
    {236, -1, -12, -5, 2, -2, -3, 1},
    {-23, -17, -6, -3, -3, 0, 0, -1},
    {-11, -9, -2, 2, 0, -1, -1, 0},
    {-7, -2, 0, 1, 1, 0, 0, 0},
    {-1, -1, 1, 2, 0, -1, 1, 1},
    {2, 0, 2, 0, -1, 1, 1, -1},
    {-1, 0, 0, -1, 0, 2, 1, -1},
    {-3, 2, -4, -2, 2, 1, -1, 0},
    };

    105 200 254 253 1 1 0 0
        30 241 249 1 0 1 0 0
        6 2 0 255 0 0 0 0
        4 254 1 0 0 255 0 0
        1 1 0 0 0 0 0 0
        1 0 0 0 0 0 0 0
        0 0 0 0 0 0 0 0
        0 0 0 0 0 0 0 0
*/
    /*float** block = create_2D_array(8);
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            block[i][j] = temp_block[i][j];
        }
    }*/
    //quantize(block, 50);
    //invert_quantize(block, 50);
    //float** block = create_2D_array(8);
    //for (int i = 0; i < 8; i++) {
    //    for (int j = 0; j < 8; j++) {
    //        block[i][j] = 0;
    //    }
    //}    
    //// example 1
    //block[0][0] = 15;
    //block[0][1] = -2;
    //block[0][2] = -1;
    //block[1][1] = -1;
    //block[2][0] = -1;
    //block[1][2] = -1;
    //// example 2
    //block[0][0] = 5;
    //block[0][1] = 2;
    //block[0][2] = 1;
    //block[1][1] = -1;
    //block[1][0] = -3;
    //block[1][3] = -1;
    //block[2][0] = 1;
    
    //compressed_bitstream = "10010101000110001000001111101001010";
    /*compressed_bitstream = "1111111100111101001010";

    float** block = create_2D_array(8);
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            block[i][j] = 0;
        }
    }    
    block[1][4] = -1;*/
    /*vector<SnakeBody> snake_vec = AC_run_length(block);
    string ac_codewords = AC_encode(snake_vec);*/

    //int last_DC = 34;
    //get_invert_tables();
    //// DC decode
    //int DC = dc_decode(last_DC);
    //last_DC = DC;

    //// AC decode
    //vector<SnakeBody> ac_snake = ac_decode();
    //float** block2 = invert_AC_run_length(0, snake_vec);
    //for (int i = 0; i < 8; i++) {
    //    for (int j = 0; j < 8; j++) {
    //        cout << round(block2[i][j])<<" ";
    //    }
    //    cout << endl;
    //}
    //string ac_codewords = AC_encode(ac_snake);
    //string dc_codewords = DC_encode(0);
    //dc_codewords = DC_encode(0);

    //// AC_run_length decode
    //block = invert_AC_run_length(DC, ac_snake);

    // invert dct processing
    //idct2(block, 8);
    //for (int i = 0; i < 8; i++) {
    //    for (int j = 0; j < 8; j++) {
    //        cout << round(block[i][j])<<" ";
    //    }
    //    cout << endl;
    //}    
    //cout << endl;
    //dct2(block, 8);
    //for (int i = 0; i < 8; i++) {
    //    for (int j = 0; j < 8; j++) {
    //        cout << round(block[i][j]) << " ";
    //    }
    //    cout << endl;
    //}


    /*vector<SnakeBody> snake_vec = AC_run_length(block);
    string ac_codewords = AC_encode(snake_vec);
    string dc_codewords = DC_encode(block[0][0]);
    string codewords = dc_codewords + ac_codewords;*/

    int choice;
    string img_name;
    string process_type;
    string data_path = "./Test Images/";

    cout << "Lena: 1" << endl;
    cout << "Baboon: 2" << endl;
    cout << "Please choose the image: ";
    cin >> choice;
    cout << endl;
    //choice = 1;

    if (choice == 1) {
        img_name = "Lena";
    }
    else if (choice == 2) {
        img_name = "Baboon";
    }
    else {
        cerr << "Image doesn't exist!" << endl;
        return 1;
    }

    cout << "Gray: 1" << endl;
    cout << "RGB: 2" << endl;
    cout << "Please choose the color type: ";
    cin >> choice;
    cout << endl;
    //choice = 2;

    if (choice == 1) {
        process_type = "";
        data_path += "GrayImages/";
    }
    else if (choice == 2) {
        process_type = "RGB";
        data_path += "ColorImages/";
    }
    else {
        cerr << "Image doesn't exist!" << endl;
        return 1;
    }

    //根據使用者選擇的檔案名稱和處理方式執行相應的處理
    string raw_name = data_path + img_name + process_type + ".raw"; //"./Data/RAW/lena_decode.raw";
    cout << raw_name << " Loading..." << endl;

    int n = 8;
    int image_width = 512;
    get_invert_tables();

    if (choice != 1) {
        float*** original_color_img = read_color_raw_img(raw_name);
        float*** YCbCr_img = toYCbCr_img(original_color_img);
        
        //write_gray_img(YCbCr_img[0], img_name + process_type + "_Y.raw", 512);
        //write_gray_img(YCbCr_img[1], img_name + process_type + "_Cb.raw", 512);
        //write_gray_img(YCbCr_img[2], img_name + process_type + "_Cr.raw", 512);
        //float*** RGB_img0 = toRGB_img(YCbCr_img);
        //write_color_img(RGB_img0, img_name + process_type + "_RGB.raw", 512);


        // subsampling mode
        cout << "4:4:4 mode: 1" << endl;
        cout << "4:2:2 mode: 2" << endl;
        cout << "4:1:1 mode: 3" << endl;
        cout << "Please choose the Subsampling mode: ";
        cin >> choice;
        cout << endl;
        //choice = 3;
        string subsampling_mode;

        if (choice == 2) {
            subsampling_mode = "422";
            subsampling(YCbCr_img, subsampling_mode);
            //write_gray_img(YCbCr_img[1], img_name + process_type + "_Cb_" + subsampling_mode + ".raw", 512);
            //write_gray_img(YCbCr_img[2], img_name + process_type + "_Cr_" + subsampling_mode + ".raw", 512);
            //float*** RGB_img = toRGB_img(YCbCr_img);
            //write_color_img(RGB_img, img_name + process_type + "_RGB_" + subsampling_mode + ".raw", 512);
        }
        else if (choice == 3) {
            subsampling_mode = "411";
            subsampling(YCbCr_img, subsampling_mode);
        }  

        // encoding, scan 512 * 512 with 8 * 8 DCT
        for (const int QF : {5, 10, 20, 50, 80, 90}) {
            vector<vector<SnakeBody>> all_snakes_encode;
            vector<vector<SnakeBody>> all_snakes_decode;
            vector<vector<char>> all_blocks_encode;
            vector<vector<char>> all_blocks_decode;

            
            string bitstream = "";
            float*** processed_img = create_3D_array(image_width);

            for (int c : {0, 1, 2}) {
                int x_pos = 0, y_pos = 0;
                int last_DC = 0;
                while (y_pos < image_width) {
                    // create empty 8*8 block
                    float** block = create_2D_array(n);

                    //float** block = new float* [8];
                    //for (int i = 0; i < 8; ++i) {
                    //    block[i] = new float[8]; // 为每行创建 float 数组
                    //}
                    // scan 8*8 block from 512*512 image
                    for (int i = 0; i < 8; i++) {
                        for (int j = 0; j < 8; j++) {
                            block[i][j] = YCbCr_img[c][x_pos + i][y_pos + j] - 128;
                        }
                    }

                    // dct processing
                    dct2(block, n);
                    // Quantization
                    quantize(block, QF, c);

                    // DC encode
                    int diff_DC = block[0][0] - last_DC;
                    last_DC = block[0][0];
                    string dc_codewords = DC_encode(diff_DC);

                    // save block for debugging
                    vector <char> temp_block(64);
                    for (int i = 0; i < 8; i++) {
                        for (int j = 0; j < 8; j++) {
                            temp_block[i * 8 + j] = block[i][j];
                        }
                    }
                    all_blocks_encode.push_back(temp_block);

                    // AC encode
                    vector<SnakeBody> snake_vec = AC_run_length(block);
                    string ac_codewords = AC_encode(snake_vec);
                    //cout << snake_vec.size()<<endl;
                    /*for (int i = 0; i < snake_vec.size();i++) {
                        cout << "<"<<int(snake_vec[i].zeros) << ", " << int(snake_vec[i].value) <<"> ";
                    }
                    cout << "EOF" << endl;*/
                    all_snakes_encode.push_back(snake_vec);
                    string codewords = dc_codewords + ac_codewords;
                    bitstream += codewords;

                    // push 8*8 block to processed image
                    for (int i = 0; i < 8; i++) {
                        for (int j = 0; j < 8; j++) {
                            processed_img[c][x_pos + i][y_pos + j] = block[i][j];
                        }
                    }

                    // move to next 8*8 position
                    x_pos += 8;
                    if (x_pos == image_width) {
                        y_pos += 8;
                        x_pos = 0;
                    }
                }
            }

            // save as .hahajpg
            string filename = img_name + process_type + "_QF" + to_string(QF) + ".hahajpg";
            cout << filename << endl;
            //cout << "length of bitstream = " << bitstream.length() << endl;// << bitstream << endl;
            write_jpg(filename, bitstream);

            // calculate PSNR, QF=90, 80, 50, 20, 10 and 5
            //double PSNR = calculatePSNR(YCbCr_img, processed_img, image_width);
            //printf("QF = %d, PSNR = %lf\n", QF, PSNR);

            // compare the file size
            size_t uncompressed_file_size = image_width * image_width * 3; // 65536
            size_t compressed_file_size = bitstream.length() / 8; // 499680 / 8 = 62460

            int uncompressed_file_kb = roundf(float(uncompressed_file_size) / 1024); //64KB
            int compressed_file_kb = roundf(float(compressed_file_size) / 1024); //60KB
            printf("original file size: %zd Bytes (= %d KB). \n", uncompressed_file_size, uncompressed_file_kb);
            printf("compressed size is: %zd Bytes (= %d KB).\n", compressed_file_size, compressed_file_kb);
            printf("Space saving: %0.2f%% \n", float(uncompressed_file_kb - compressed_file_kb) / float(uncompressed_file_kb) * 100);

            // decode
            //string compressed_bitstream = read_jpg(filename);
            compressed_bitstream = read_jpg(filename);
            // int stream_len = compressed_bitstream.length();

            // cout << "length of compressed_bitstream = " << stream_len << endl;// << bitstream << endl;
            // cout << compressed_bitstream.compare(bitstream) << endl; // the same

            float*** uncompressed_img = create_3D_array(image_width);

            for (int c : {0, 1, 2}) {
                int x_pos = 0, y_pos = 0;
                int last_DC = 0;

                while (y_pos < image_width) {
                    int bit_num = 4;
                    int category;
                    // create empty 8*8 block
                    float** block = create_2D_array(n);

                    // DC decode
                    int DC = dc_decode(last_DC);
                    last_DC = DC;

                    // AC decode
                    vector<SnakeBody> ac_snake = ac_decode();
                    all_snakes_decode.push_back(ac_snake);


                    // AC_run_length decode
                    block = invert_AC_run_length(DC, ac_snake);

                    // save block for debugging
                    vector <char> temp_block(64);
                    for (int i = 0; i < 8; i++) {
                        for (int j = 0; j < 8; j++) {
                            temp_block[i * 8 + j] = block[i][j];
                        }
                    }
                    all_blocks_decode.push_back(temp_block);

                    // invert Quantization
                    invert_quantize(block, QF, c);
                    // invert dct processing
                    idct2(block, n);

                    // push 8*8 block to uncompressed image
                    for (int i = 0; i < n; i++) {
                        for (int j = 0; j < n; j++) {
                            uncompressed_img[c][x_pos + i][y_pos + j] = round(block[i][j] + 128);
                        }
                    }

                    // move to next 8*8 position
                    x_pos += n;
                    if (x_pos == image_width) {
                        y_pos += n;
                        x_pos = 0;
                        //cout << y_pos << " ";
                    }
                }
            }
            // write uncompressed_img to .raw
            string decode_raw_name = img_name + process_type + "_QF" + to_string(QF) + "_decoded.raw";
            float*** RGB_img = toRGB_img(uncompressed_img);
            write_color_img(RGB_img, decode_raw_name, image_width);

            // calculate PSNR, QF=90, 80, 50, 20, 10 and 5
            double PSNR = calculatePSNR(original_color_img, RGB_img, image_width);
            printf("QF = %d, PSNR = %lf\n", QF, PSNR);
            cout << endl << "output path: " << decode_raw_name << endl << endl;

        }

    }
    else {
        // Gray level
        float** original_img = read_raw_img(raw_name);

        // encoding, scan 512 * 512 with 8 * 8 DCT
        for (const int QF : {5, 10, 20, 50, 80, 90}) {
            vector<vector<SnakeBody>> all_snakes_encode;
            vector<vector<SnakeBody>> all_snakes_decode;
            vector<vector<char>> all_blocks_encode;
            vector<vector<char>> all_blocks_decode;

            int x_pos = 0, y_pos = 0;
            int last_DC = 0;
            string bitstream = "";
            float** processed_img = create_2D_array(image_width);

            while (y_pos < image_width) {
                // create empty 8*8 block
                float** block = create_2D_array(n);

                //float** block = new float* [8];
                //for (int i = 0; i < 8; ++i) {
                //    block[i] = new float[8]; // 为每行创建 float 数组
                //}
                // scan 8*8 block from 512*512 image
                for (int i = 0; i < 8; i++) {
                    for (int j = 0; j < 8; j++) {
                        block[i][j] = original_img[x_pos + i][y_pos + j] - 128;
                    }
                }

                // dct processing
                dct2(block, n);
                // Quantization
                quantize(block, QF, 0);

                // DC encode
                int diff_DC = block[0][0] - last_DC;
                last_DC = block[0][0];
                string dc_codewords = DC_encode(diff_DC);

                // save block for debugging
                vector <char> temp_block(64);
                for (int i = 0; i < 8; i++) {
                    for (int j = 0; j < 8; j++) {
                        temp_block[i * 8 + j] = block[i][j];
                    }
                }
                all_blocks_encode.push_back(temp_block);

                // AC encode
                vector<SnakeBody> snake_vec = AC_run_length(block);
                string ac_codewords = AC_encode(snake_vec);
                //cout << snake_vec.size()<<endl;
                /*for (int i = 0; i < snake_vec.size();i++) {
                    cout << "<"<<int(snake_vec[i].zeros) << ", " << int(snake_vec[i].value) <<"> ";
                }
                cout << "EOF" << endl;*/
                all_snakes_encode.push_back(snake_vec);
                string codewords = dc_codewords + ac_codewords;
                bitstream += codewords;

                // push 8*8 block to processed image
                for (int i = 0; i < 8; i++) {
                    for (int j = 0; j < 8; j++) {
                        processed_img[x_pos + i][y_pos + j] = block[i][j];
                    }
                }

                // move to next 8*8 position
                x_pos += 8;
                if (x_pos == image_width) {
                    y_pos += 8;
                    x_pos = 0;
                }
            }

            // save as .hahajpg
            string filename = img_name + process_type + "_QF" + to_string(QF) + ".hahajpg";
            cout << filename << endl;
            //cout << "length of bitstream = " << bitstream.length() << endl;// << bitstream << endl;
            write_jpg(filename, bitstream);

            // calculate PSNR, QF=90, 80, 50, 20, 10 and 5
            //double PSNR = calculatePSNR(original_img, processed_img, image_width);
            //printf("QF = %d, PSNR = %lf\n", QF, PSNR);

            // compare the file size
            size_t uncompressed_file_size = image_width * image_width; // 65536
            size_t compressed_file_size = bitstream.length() / 8; // 499680 / 8 = 62460

            int uncompressed_file_kb = roundf(float(uncompressed_file_size) / 1024); //64KB
            int compressed_file_kb = roundf(float(compressed_file_size) / 1024); //60KB
            printf("original file size: %zd Bytes (= %d KB). \n", uncompressed_file_size, uncompressed_file_kb);
            printf("compressed size is: %zd Bytes (= %d KB).\n", compressed_file_size, compressed_file_kb);
            printf("Space saving: %0.2f%% \n", float(uncompressed_file_kb - compressed_file_kb) / float(uncompressed_file_kb) * 100);

            // decode
            //string compressed_bitstream = read_jpg(filename);
            compressed_bitstream = read_jpg(filename);
            // int stream_len = compressed_bitstream.length();

            // cout << "length of compressed_bitstream = " << stream_len << endl;// << bitstream << endl;
            // cout << compressed_bitstream.compare(bitstream) << endl; // the same

            float** uncompressed_img = create_2D_array(image_width);

            x_pos = 0, y_pos = 0;
            last_DC = 0;

            while (y_pos < image_width) {
                int bit_num = 4;
                int category;
                // create empty 8*8 block
                float** block = create_2D_array(n);

                // DC decode
                int DC = dc_decode(last_DC);
                last_DC = DC;

                // AC decode
                vector<SnakeBody> ac_snake = ac_decode();
                all_snakes_decode.push_back(ac_snake);


                // AC_run_length decode
                block = invert_AC_run_length(DC, ac_snake);

                // save block for debugging
                vector <char> temp_block(64);
                for (int i = 0; i < 8; i++) {
                    for (int j = 0; j < 8; j++) {
                        temp_block[i * 8 + j] = block[i][j];
                    }
                }
                all_blocks_decode.push_back(temp_block);

                // invert Quantization
                invert_quantize(block, QF, 0);
                // invert dct processing
                idct2(block, n);

                // push 8*8 block to uncompressed image
                for (int i = 0; i < n; i++) {
                    for (int j = 0; j < n; j++) {
                        uncompressed_img[x_pos + i][y_pos + j] = round(block[i][j] + 128);
                    }
                }

                // move to next 8*8 position
                x_pos += n;
                if (x_pos == image_width) {
                    y_pos += n;
                    x_pos = 0;
                    //cout << y_pos << " ";
                }
            }
            // write uncompressed_img to .raw
            string decode_raw_name = img_name + process_type + "_QF" + to_string(QF) + "_decoded.raw";
            write_gray_img(uncompressed_img, decode_raw_name, image_width);
            // calculate PSNR, QF=90, 80, 50, 20, 10 and 5
            double PSNR = calculatePSNR(original_img, uncompressed_img, image_width);
            printf("QF = %d, PSNR = %lf\n", QF, PSNR);
            cout << endl << "output path: " << decode_raw_name << endl << endl;

            // compare snakes
        //    for (int i = 0; i < 64; i++) {
        //        if (!(all_snakes_decode[i] == all_snakes_encode[i])) {
        //            cout << endl << "decoder snake:" << endl;
        //            cout << "size: " << all_snakes_decode[i].size() << endl;
        //            for (int j = 0; j < all_snakes_decode[i].size(); j++) {
        //                cout << "<" << int(all_snakes_decode[i][j].zeros) << ", " << int(all_snakes_decode[i][j].value) << "> ";
        //            }
        //            cout << endl;
        //            cout << "encoder snake:" << endl;
        //            cout << "size: " << all_snakes_encode[i].size() << endl;
        //            for (int j = 0; j < all_snakes_encode[i].size(); j++) {
        //                cout << "<" << int(all_snakes_encode[i][j].zeros) << ", " << int(all_snakes_encode[i][j].value) << "> ";
        //            }
        //            cout << endl;
        //        }
        //        else {
        //            cout << "true ";
        //        }
        //    }
        //    cout << endl;
        //    //compare blocks
        //    for (int i = 0; i < 64; i++) {
        //        if (!(all_blocks_decode[i] == all_blocks_encode[i])) {
        //            cout << "encoder snake:" << endl;
        //            cout << "size: " << all_snakes_encode[i].size() << endl;
        //            for (int j = 0; j < all_snakes_encode[i].size(); j++) {
        //                cout << "<" << int(all_snakes_encode[i][j].zeros) << ", " << int(all_snakes_encode[i][j].value) << "> ";
        //            }
        //            cout << endl << "decoder block:" << endl;
        //            for (int k = 0; k < 8; k++) {
        //                for (int j = 0; j < 8; j++) {
        //                    cout << round(all_blocks_decode[i][k * 8 + j]) << " ";
        //                }
        //                cout << endl;
        //            }
        //            cout << endl;
        //            cout << "encoder block:" << endl;
        //            for (int k = 0; k < 8; k++) {
        //                for (int j = 0; j < 8; j++) {
        //                    cout << round(all_blocks_encode[i][k * 8 + j]) << " ";
        //                }
        //                cout << endl;
        //            }
        //            cout << endl;
        //        }
        //        else {
        //            cout << "true ";
        //        }

        //    }
        //    cout << endl;
        }
        cout << endl;


    }

    return 0;
}