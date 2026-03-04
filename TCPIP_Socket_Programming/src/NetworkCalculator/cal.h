#ifndef CAL_H
#define CAL_H

#pragma pack(push, 1)
struct cal_data {
	int left_num;
	int right_num;
	char op;
	int result;
	int error;
};
#pragma pack(pop)

// 계산 로직을 헤더에 내장
static inline void calculate(struct cal_data* data) {
    data->error = 0; // 초기화
    data->result = 0;

    switch (data->op) {
    case '+':
        data->result = data->left_num + data->right_num;
        break;
    case '-':
        data->result = data->left_num - data->right_num;
        break;
    case '*':
        data->result = data->left_num * data->right_num;
        break;
    case '/':
        if (data->right_num != 0) {
            data->result = data->left_num / data->right_num;
        }
        else {
            data->error = 1; // Division by zero
        }
        break;
    default:
        data->error = 2; // Unknown operator
        break;
    }
}

#endif