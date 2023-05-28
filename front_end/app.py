from flask import Flask, render_template, jsonify
import json

app = Flask(__name__)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/get_data', methods=['GET'])
def get_data():
    # 从JSON文件中读取数据
    with open('D:\\Codes\\CN_1\\data\\flow_data.json', 'r', encoding='utf-8') as f:
        data = json.load(f)['data']
    
    # name_mapping = {
    #     '北京': '北京市',
    #     '天津': '天津市',
    #     '上海': '上海市',
    #     '重庆': '重庆市',
    #     '澳门': '澳门特别行政区',
    #     '香港': '香港特别行政区',
    #     '新疆': '新疆维吾尔自治区',
    #     '西藏': '西藏自治区',
    #     '宁夏': '宁夏回族自治区',
    #     '广西': '广西壮族自治区',
    #     '内蒙古': '内蒙古自治区',
    # }
    # converted_data = [(name_mapping.get(item['name'], item['name']), item['value']) for item in data]
    # 返回数据的JSON响应
    return jsonify(data)

if __name__ == '__main__':
    app.run()