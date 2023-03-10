namespace arps {
    
// direction - to be implemented later
enum direction {
    up = 0,
    down,
    upUp,
    downDown,
    upDown,
    downUp,
    random,
    LAST_DIR
};
// 0 scale
const float noMajTri[3] = {0, 4, 7};
const float noMinTri[3] = {0, 3, 7};
const float noMajPen[5] = {0, 2, 4, 5, 7};
const float noMinPen[5] = {0, 2, 3, 5, 7};

// triples
//const float majTri[3] = {48.0f, 52.0f, 55.0f};
//const float minTri[3] = {48.0f, 51.0f, 55.0f};
//
// fifths
//const float majFif[5] = {48.0f, 50.0f, 52.0f, 53.0f, 55.0f};
//const float minFif[5] = {48.0f, 50.0f, 51.0f, 53.0f, 55.0f};

} // namespace arps
