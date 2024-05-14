#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <map>
#include <bitset>

#include "dct2.h"

using namespace std;


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
    float** original_img = new float* [n]; // 创建指向 float 指针的指针

    for (int i = 0; i < n; ++i) {
        original_img[i] = new float[n]; // 为每行创建 float 数组
    }

    // 讀取 vector 數據到 float array
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            original_img[i][j] = temp_vec[i*n+j];
        }
    }

    cout << "已成功讀取 " << int(fileSize) << " 個字節的數據" << endl << endl;

    // 關閉檔案
    rawFile.close();

    return original_img;
}

void quantize(float** x, int QF) {
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
    float factor;
    if (QF < 50){
        factor = 5000 / QF;
    }
    else {
        factor = 200 - 2 * QF;
    }
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            float qij = QUAN_MATRIX[i][j] * factor / 100;
            x[i][j] = round(x[i][j] / qij);
        }
    }
}

struct SnakeBody {
    int zeros;
    float value;
    SnakeBody(int z, float v) { zeros = z; value = v; };
};

vector<SnakeBody> AC_run_length(float** block) {
    vector<SnakeBody> snake_vec;
    int sign = -1;
    int i = 1, j = 0;
    int time_increse = 1;
    int zeros = 0;
    for (int times = 1; times <= 7 && times >=0 ; times+=time_increse, sign *= -1) {
        for (int time_count = times; time_count >= 0 ;time_count--, i += sign, j -= sign) {
            if (block[i][j] == 0) {
                zeros++;
            }
            else {
                snake_vec.push_back(SnakeBody(zeros, block[i][j]));
                zeros = 0;
            }
        }
        i -= sign;
        j += sign;
        
        if (times == 7){
            time_increse = -1;
        }
        if (sign == 1 && time_increse == 1 || sign == -1 && time_increse == -1) {
            i += 1;
        }
        else if(sign == -1 && time_increse == 1 || sign == 1 && time_increse == -1) {
            j += 1;
        }

    }
    if (block[7][7] == 0){
        snake_vec.push_back(SnakeBody(-1, EOF));
    }
    return snake_vec;
}


struct ACData {
    int run;
    int category;
    string codeword;
};


// 函數用於解析 CSV 檔案中的一行數據並返回一個結構
vector<ACData> parseCSV(string filename) {

    // 打開 CSV 檔案
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "無法打開檔案" << endl;
    }

    vector<ACData> dataList; // 存儲所有行數據的向量

    // 逐行讀取檔案內容
    string line;
    while (getline(file, line)) {
        // 解析每一行並將數據存儲到 dataList 向量中
        ACData data;
        stringstream ss(line);
        string token;

        // 逐一讀取每個逗號分隔的欄位
        getline(ss, token, ',');
        data.run = stoi(token);
        getline(ss, token, ',');
        data.category = stoi(token);
        getline(ss, token, ',');
        data.codeword = token;
        dataList.push_back(data);
    }
    file.close();

    return dataList;
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

string AC_encode(vector<SnakeBody> snake_vec) {
    //map<int, string> conversionMap;
    //// 建立整數到二進制字符串的映射
    //for (int i = -15; i <= 15; ++i) {
    //    conversionMap[i] = intToBinaryString(i);
    //}

    //// 輸出映射結果
    //for (const auto& pair : conversionMap) {
    //    cout << pair.first << " -> " << pair.second << endl;
    //}

    string bitstring="";
    string filename = "AC_Luminance.csv";
    vector<ACData> AC_table = parseCSV(filename);
    for (SnakeBody s : snake_vec) {
        string codeword;
        int category = floor(log(s.value)) + 1;
        if (s.zeros == -1) {
            codeword = "1010";
        }
        else {
            string coefficient_codeword = intToBinaryString(s.value);
            codeword = AC_table[s.zeros * 10 + category].codeword + coefficient_codeword;
        }
        bitstring += codeword;
    }

    return bitstring;
}


string DC_encode(int diff_DC) {
    string dc_codewords;
    return dc_codewords;
}


int main() {
    int choice;
    string img_name;
    string process_type;
    string data_path = "./Test Images/";

    cout << "Lena: 1" << endl;
    cout << "Baboon: 2" << endl;
    cout << "Please choose the image: ";
    //cin >> choice;
    cout << endl;
    choice = 1;

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
    //cin >> choice;
    cout << endl;
    choice = 1;

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

    float **original_img = read_raw_img(raw_name);
    int n = 8;
    cout << original_img[255][256] << endl;
    cout << original_img[256][256] << endl;

    // scan 512 * 512 with 8 * 8 DCT
    int x_pos = 0, y_pos = 0;
    const int QF = 50;
    int last_DC = 0;

    while (y_pos < 512) {
        // create empty 8*8 block
        float** block = new float* [8];
        for (int i = 0; i < 8; ++i) {
            block[i] = new float[8]; // 为每行创建 float 数组
        }
        // scan 8*8 block from 512*512 image
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                block[i][j] = original_img[x_pos + i][y_pos + j];
            }
        }
        // dct processing
        dct2(block, n);
        // Quantization
        quantize(block, QF);

        int diff_DC = block[0][0]-last_DC;
        last_DC = block[0][0];
        vector<SnakeBody> snake_vec = AC_run_length(block);
        string codewords = AC_encode(snake_vec);
        //cout << snake_vec.size()<<endl;
        /*for (int i = 0; i < snake_vec.size();i++) {
            cout << int(snake_vec[i].zeros) << " " << int(snake_vec[i].value) << endl;
        }*/

        // DC encode
        string dc_codewords = DC_encode(diff_DC);


        // push 8*8 block back to original image
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                original_img[x_pos + i][y_pos + j] = block[i][j];
            }
        }
        // move to next 8*8 position
        x_pos += 8;
        if (x_pos == 512) {
            y_pos += 8;
            x_pos = 0;
        }
    }
    
    cout << original_img[255][256] << endl;
    cout << original_img[256][256] << endl;

	return 0;
}