#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node {
    char *data;
    int registerNumber;
    int isLeaf;
    struct node *left;
    struct node *right;
} Node;

int registers[10];
char *resultName;
Node *root;

Node *newNode() {
    // Allocate memory for new node
    Node *node = (struct node *) malloc(sizeof(struct node));
    // Initialize left and
    // right children as NULL
    node->data = NULL;
    node->left = NULL;
    node->right = NULL;
    node->isLeaf = 0;
    node->registerNumber = -1;
    return (node);

}

char *substring(char *string, int position, int length) {
    char *p;
    int c;
    p = malloc(length + 1);
    if (p == NULL) {
        printf("Unable to allocate memory.\n");
        exit(1);
    }

    for (c = 0; c < length; c++) {
        *(p + c) = *(string + position - 1);
        string++;
    }

    *(p + c) = '\0';

    return p;
}

char *delBrackets(char *name) {
    int start = 0;
    while (start < strlen(name) && name[start] == '(' || name[start] == ')') {
        start++;
    }
    int end = strlen(name) - 1;
    while (end >= 0 && name[end] == ')' || name[end] == '(') {
        end--;
    }
    return substring(name, start + 1, end - start + 1);
}

void initializeRoot() {
    root = newNode();
}

void setResultName(char *name) {
    resultName = name;
}

int getPriority(char operation) {
    switch (operation) {
        case '+':
        case '-':
            return 2;
        case '*':
        case '/':
        case '%':
            return 1;
        default:
            return -101;
    }
}
//strcmp-comparator for char*

char *getCommand(char *key) {
    if (strcmp(key, "+") == 0) {
        return "add";
    } else if (strcmp(key, "-") == 0) {
        return "sub";
    } else if (strcmp(key, "/") == 0) {
        return "div";
    } else if (strcmp(key, "*") == 0) {
        return "mult";
    } else if (strcmp(key, "%") == 0) {
        return "mod";
    } else {
        printf("%s", key);
        exit(1);
    }
}

int firstFreeRegister() {
    for (int i = 0; i < sizeof(registers) / sizeof(registers[0]); i++) {
        if (registers[i] == 0) {
            registers[i] = 1;
            return i;
        }
    }
    return -1;
}

void print() {
    printf("Iam here!");
}

void makeTree(char *expression, Node *current, int start, int finish) {
    int minPriority = -100;
    int index = -1;
    int bracketsCount = 0;
    int singleMinus;
    for (int i = start; i < finish; i++) {
        if (expression[i] == '(') {
            bracketsCount++;
        } else if (expression[i] == ')') {
            bracketsCount--;
        } else {
            singleMinus = expression[i] == '-' && (i != start && getPriority(expression[i - 1]) == 1);
            if (!singleMinus && getPriority(expression[i]) - bracketsCount * 2 >= minPriority) {
                minPriority = getPriority(expression[i]) - bracketsCount * 2;
                index = i;
            }
        }
    }
    if (index == -1) {
        current->data = delBrackets(substring(expression, start + 1, finish - start));
        current->isLeaf = 1;
    }
    if (index != -1) {
        current->data = substring(expression, index + 1, 1);
        Node *left = newNode();
        current->left = left;
        if (index != start) { //processing minus sign '-'
            makeTree(expression, left, start, index);
        } else {
            left->data = "0";
        }
        if (index != finish - 1) {
            Node *right = newNode();
            current->right = right;
            makeTree(expression, right, index + 1, finish);
        }
    }
}

void buildTree(char *expression) {
    initializeRoot();
    makeTree(expression, root, 0, strlen(expression));
}

void processLeaf(Node *current) {
    char *command = getCommand(current->data);
    int regNum = firstFreeRegister();
    if (regNum == -1) {
        printf("Нет свободного регистра");
        exit(1);
    }
    printf("mov ax %s\n", current->left->data);
    printf("%s ax %s\n", command, current->right->data);
    printf("mov T%d ax\n", regNum);
    current->registerNumber = regNum;
}

void i86Print(Node *current) {
    Node *left = current->left;
    Node *right = current->right;
    int leftLeaf = left->isLeaf;
    int rightLeaf = right->isLeaf;
    char *command = getCommand(current->data);
    if (!leftLeaf && rightLeaf) {
        printf("mov ax T%d\n", left->registerNumber);
        printf("%s ax %s\n", command, right->data);
        printf("mov T%d ax\n", left->registerNumber);
        current->registerNumber = left->registerNumber;
    } else if (leftLeaf && !rightLeaf) {
        printf("mov ax %s\n", left->data);
        printf("%s ax T%d\n", command, right->registerNumber);
        printf("mov T%d ax\n", right->registerNumber);
        current->registerNumber = right->registerNumber;
    } else if (rightLeaf && leftLeaf) {
        printf("mov ax %s\n", left->data);
        printf("%s ax %s\n", command, right->data);
        int reg = firstFreeRegister();
        printf("mov T%d ax\n", reg);
        current->registerNumber = reg;
    } else {
        printf("mov ax T%d\n", left->registerNumber);
        printf("%s ax T%d\n", command, right->registerNumber);
        printf("mov T%d ax\n", left->registerNumber);
        current->registerNumber = left->registerNumber;
        registers[right->registerNumber] = 0;
    }
}

void printTree(Node *current) {
    Node *left = current->left;
    Node *right = current->right;
    if (left == NULL || right == NULL) {
        current->isLeaf = 1;
        return;
    }
    if (left->isLeaf == 1 && right->isLeaf == 1) {
        processLeaf(current);
        if (current == root) {
            printf("mov %s T%d\n", resultName, current->registerNumber);
        }
        return;
    }
    printTree(left);
    printTree(right);
    i86Print(current);
    if (current == root) {
        printf("mov %s T%d\n", resultName, current->registerNumber);
    }
}

void printSyntaxTree() {
    printTree(root);
}

int main() {
    char *expression = "a-b-c-d*-1";
    expression = "aab*asd+32/12*sdd+11+1+4%2";
    expression = "1*aa+c+1+2+3%df*asd+zx-a+1311-0";
    expression = "(13+(12)*-1*(((1+a)*(ba+23))))*-1";
    buildTree(expression);
    setResultName("result");
    printSyntaxTree();
    return 0;
}
