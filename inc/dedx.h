#include <vector>

int main(int argc, char **argv);

// Quadtratic formula stuff
std::vector<double> quadraticFormula(double a, double b, double c);
double getDiscriminant(double a, double b, double c);
double calculateA();
double calculateB(double muonToHit[3], double muonEntry[3], double HitTime, double muonEntryTime);
double calculateC(double magnitudeR, double hitTime, double muonEntryTime);
double calculateZ(double longDist, double muonToHit[3], double muonDir[3]);
double calculateT(double longDist, double muonToHit[3], double muonDir[3]);

constexpr double cVac = 29.9792458; // cm/ns
constexpr double ng = 1.38;         // Group refractive index in water for Cherenkov light
