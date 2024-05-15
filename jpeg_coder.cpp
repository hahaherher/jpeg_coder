#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <string>
#include <map>
#include <bitset>

#include "dct2.h"

using namespace std;

float** create_2D_array(int n) {
    // 二维 float array
    float** array_2D = new float* [n]; // 创建指向 float 指针的指针
    for (int i = 0; i < n; ++i) {
        array_2D[i] = new float[n]; // 为每行创建 float 数组
    }

    return array_2D;
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
        vector<int> ac_indices(2);
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

void quantize(float** x, int QF) {
    
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
    snake_vec.push_back(SnakeBody(-1, EOF));
    
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


map<vector<int>, string> AC_table = parseCSV("AC_Luminance.csv");

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

    for (SnakeBody s : snake_vec) {
        string codeword;
        int category = floor(log(abs(s.value))) + 1;
        if (s.zeros == -1) {
            codeword = "1010";
        }
        else {
            string coefficient_codeword = intToBinaryString(s.value);
            vector<int> ac_indices = { s.zeros , category };
            codeword = AC_table[ac_indices] + coefficient_codeword;
        }
        bitstring += codeword;
    }

    return bitstring;
}


string DC_table[12] = {
    // DC Luminance table
    "00", "010", "011", "100", "101", "110",
    "1110", "11110", "111110", "1111110", "11111110", "111111110"
};

string DC_encode(int diff_DC) {
    string dc_codewords;
    string coefficient_codeword = intToBinaryString(diff_DC);
    int category = (diff_DC == 0) ? 0 : floor(log(abs(diff_DC))) + 1;

    dc_codewords = DC_table[category] + coefficient_codeword;

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
        cout << "Binary data has been written to file.\n" << endl;
    }
    else {
        cerr << "Unable to open file." << endl;
    }
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

    float** original_img = read_raw_img(raw_name);

    int n = 8;
    int image_width = 512;
    //cout << original_img[255][256] << endl;
    //cout << original_img[256][256] << endl;

    // scan 512 * 512 with 8 * 8 DCT
    for (const int QF : {90, 80, 50, 20, 10, 5}) {
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
                    block[i][j] = original_img[x_pos + i][y_pos + j];
                }
            }
            // dct processing
            dct2(block, n);
            // Quantization
            quantize(block, QF);

            // DC encode
            int diff_DC = block[0][0] - last_DC;
            last_DC = block[0][0];
            string dc_codewords = DC_encode(diff_DC);

            // AC encode
            vector<SnakeBody> snake_vec = AC_run_length(block);
            string ac_codewords = AC_encode(snake_vec);
            //cout << snake_vec.size()<<endl;
            /*for (int i = 0; i < snake_vec.size();i++) {
                cout << int(snake_vec[i].zeros) << " " << int(snake_vec[i].value) << endl;
            }*/

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

        //cout << original_img[255][256] << endl;
        //cout << original_img[256][256] << endl;
        

        // save as .hahajpg
        string filename = img_name + process_type + "_QF" + to_string(QF) + ".hahajpg";
        cout << filename << endl;         
        cout << "length of bitstream = " << bitstream.length() << endl;// << bitstream << endl;
        write_jpg(filename, bitstream);

        // calculate PSNR, QF=90, 80, 50, 20, 10 and 5
        double PSNR = calculatePSNR(original_img, processed_img, image_width);
        printf("QF = %d, PSNR = %lf\n\n", QF, PSNR);

        // compare the file size

    }

	return 0;
}