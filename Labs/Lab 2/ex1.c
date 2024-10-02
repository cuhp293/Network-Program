#include <stdio.h>

typedef struct student{
  char name[20];
  int eng;
  int math;
  int phys;
  double mean;
} STUDENT;

void calMean(STUDENT *data, int count);
void prtStu(STUDENT *data, int count);
char calGrade(double mean);

int main() {
  STUDENT data[]={
    {"Tuan", 82, 72, 58, 0.0},
    {"Nam", 77, 82, 79, 0.0},
    {"Khanh", 52, 62, 39, 0.0},
    {"Phuong", 61, 82, 88, 0.0}
  };
  int count = sizeof(data) /sizeof(data[0]);

  calMean(data, count);
  prtStu(data, count);
  return 0;
}

void calMean(STUDENT *data, int count) {
  STUDENT *p;
  for (p = data; p < data + count; p++) {
    p->mean = (p->eng + p->math + p->phys) / 3.0;
  }
}

void prtStu(STUDENT *data, int count) {
  STUDENT *p;
  for (p = data; p < data + count; p++) {
    printf("Name: %s\n", p->name);
    printf("English score: %d\n", p->eng);
    printf("Math score: %d\n", p->math);
    printf("Physic score: %d\n", p->phys);
    printf("Mean = %.2f\n", p->mean);
    printf("Grade: %c\n", calGrade(p->mean));
    printf("-----------------------------\n\n");
  }
}

char calGrade(double mean) {
  if (mean >= 90) {
    return 'S';
  } else if (mean >= 80) {
    return 'A';
  } else if (mean >= 70) {
    return 'B';
  } else if (mean >= 60) {
    return 'C';
  } else {
    return 'D';
  }
}
