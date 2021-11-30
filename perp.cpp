#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>
#include <fstream>
#include <numeric>

enum class thing_id { ring, pen };

std::ostream& operator<< ( std::ostream& os, thing_id e )
{
    switch (e) {
    case thing_id::ring: os << "ring"; break;
    case thing_id::pen: os << "pen"; break;
    default: os << ""; break; }
    return os;
}

auto roll = [](int a, int b, int n = 1000)->double{
    //srand(time(nullptr));
    return double(rand()%(b-a)+a)/n;
};

struct thing
{
    thing_id id;
    double length;
    double diameter;
    bool test ( thing_id x ) const { return id == x; }
};

class perceptron
{
    private:
         
        // factors for every input
        double length_factor;
        double diameter_factor;
            
         // added constant
        double bias;

        //
        const std::function<thing_id(double)> fn;

        //
        const double learning_speed_factor;

        const std::string parameters_file;

    public:

        perceptron ( const int size, const std::string parameters_file,
            const std::function<thing_id(double)> fn,
            const double lsf, const double rand_range ):
            length_factor(roll(-rand_range,rand_range)),
            diameter_factor(roll(-rand_range,rand_range)),
            bias(roll(-rand_range,rand_range)),
            fn(fn), learning_speed_factor(lsf),
            parameters_file(parameters_file) {}

        perceptron ( const perceptron& perp ):
            length_factor(perp.learning_speed_factor),
            diameter_factor(perp.diameter_factor),
            bias(perp.bias), fn(perp.fn),
            learning_speed_factor(perp.learning_speed_factor) {}

        thing_id operator() ( const double length, const double diameter ) const
        {
            return fn(length*length_factor+diameter*diameter_factor+bias);
        }

        void modify ( const thing& e )
        {
            double mod{((e.id == thing_id::obraczka)?-1:1)*learning_speed_factor};
            length_factor += mod*e.length;
            diameter_factor += mod*e.diameter;
            bias += mod;
        }

        void get_parameters ( std::ostream& os = std::cout ) const
        {
            os << length_factor << " " << diameter_factor << " " << bias;
        }

        void get_parameters ( std::ostream&& os ) const { get_parameters(os); }

        void save ( std::string filename ) const { get_parameters(std::ofstream(filename)); }

        void save () const { get_parameters(std::ofstream(parameters_file)); }

        void set_parameters ( std::istream& is = std::cin )
        {
            is >> length_factor;
            is >> diameter_factor;
            is >> bias;
        }

        void set_parameters ( std::istream&& is ) { set_parameters(is); }

        void load ( std::string filename ) { set_parameters(std::ifstream(filename)); }

        void load () { set_parameters(std::ifstream(parameters_file)); }

        void reset ()
        {
            std::ofstream file(parameters_file);
            file << "";
        }
};

template < typename type >
int find_index ( const std::vector<type>& v, type e )
{
    return std::find(v.begin(),v.end(),e)-v.begin();
}

template < typename type >
void print_vector ( const std::vector<type>& v, std::string sep = "\n", std::ostream& os = std::cout )
{
    for (const type& e: v) os << e << sep;
}

template < typename type >
void print_vectors (
    const std::vector<std::vector<type>>& vectors, std::string separators = "\t\n", std::ostream& os = std::cout )
    {
        for (int i{0}; i < vectors[0].size(); ++i)
        {
            for (int j{0}; j < vectors.size(); ++j)
            {
                os << vectors[j][i] << separators[j];
            }
        }
    }

std::vector<thing> generate_data ( int size )
{
    std::vector<thing> result(size);
    for (thing& e: result)
    {
        e.id = (rand()%2)?(thing_id::ring):(thing_id::pen);
        switch (e.id)
        {
            case thing_id::ring:
                e.length = rand()%14+1;
                e.diameter = rand()%40+10;
            break;
            case thing_id::pen:
                e.length = rand()%40+10;
                e.diameter = rand()%14+1;
            break;
            default:
                e.length = 0;
                e.diameter = 0;
            break;
        }
    }
    return result;
}


int network_train ( perceptron& perp, const std::vector<thing>& training_data )
{
    std::vector<bool> result(training_data.size());

    std::transform(training_data.begin(),training_data.end(),result.begin(),
        [&](thing e){return e.test(perp(e.length,e.diameter));});
    
    int counter{0};

    for (int index{find_index<bool>(result,0)}; index != result.size(); ++counter)
    {
        perp.modify(training_data[index]);

        std::transform(training_data.begin(),training_data.end(),result.begin(),
            [&](thing e){return e.test(perp(e.length,e.diameter));});
        
        index = find_index<bool>(result,0);
    }

    return counter;
}

double network_test ( const perceptron& perp, const std::vector<thing> test_data )
{
    int counter{0};
    for (const thing& e: test_data) if (e.test(perp(e.length,e.diameter))) ++counter;
    return double(counter)/test_data.size();
}

void network_present ( const perceptron& perp, const std::vector<thing> test_data )
{
    for (const thing& e: test_data)
    {
        thing_id x = perp(e.length,e.diameter);
        if (e.test(x)) std::cout << "Dobrze, typ to " << x << "\n";
        else std::cout << "Zle, typ to nie " << x << ", typ to " << e.id << "\n";
    }
}

double network_evolve ( perceptron& perp, const std::vector<thing>& evol_data, int train_data_size )
{
    double test_before{network_test(perp,evol_data)};
    std::cout << test_before << "\n";
    network_train(perp,generate_data(train_data_size));
    double test_after{network_test(perp,evol_data)};
    std::cout << test_after << "\n\n";
    return test_after-test_before;
}

// class log
// {
//     private:
    
//         perceptron perp;
    
//         int iterations;
    
//         double accuracy;

//     public:

//         log ( const perceptron& perp, int iterations, double accuracy ):
//             perp(perp), iterations(iterations), accuracy(accuracy) {}
    
//         void operator() ( std::string filename )
//         {
//             std::ofstream file(filename,std::ofstream::app);
//             perp.get_parameters(file);
//             file << iterations << " " << accuracy << "\n";
//         }
// };

// void test_log ( perceptron perp, std::string filename,
//     int training_data_size = 100, int test_data_size = 10, int test_size = 10
// ) {
//     std::ofstream file(filename);
//     file << "";
//     for (int i{0}; i < test_size; ++i) log(perp,network_train(perp,
//         generate_data(training_data_size)),
//         network_test(perp,generate_data(test_data_size)))(filename);
// }

int main ( int argc, char* argv[] )
{
    srand(time(nullptr));

    perceptron perp(2,"params.txt",[](double x)->thing_id
    {
        return (x < 0)?(thing_id::ring):(thing_id::pen);
    },
    0.00001,5);

    std::string mode(argv[1]);

    if (mode == "train")
    {
        perp.load();
        
        network_train(perp,generate_data(std::stoi(argv[2])));
        
        perp.save();
    }
    else if (mode == "test")
    {
        std::vector<double> test_results;
        
        perp.load();
        
        for (int i{0}, n{std::stoi(argv[2])}; i < n; ++i)
        
            test_results.push_back(network_test(perp,generate_data(std::stoi(argv[3]))));
        
        double mean = std::accumulate(test_results.begin(),test_results.end(),0.0)/test_results.size();
        
        std::cout << "Test results :\n";
        
        print_vector(test_results);
        
        std::cout << "Mean of results : " << mean << "\n";
    }
    else if (mode == "predict")
    {
        perp.load();
        
        for (double length, diameter; getchar() != 'x'; getchar())
        {
            system("clear");
            
            std::cout << "Set length : ";
            
            std::cin >> length;
            
            std::cout << "Set diameter : ";
            
            std::cin >> diameter;
            
            std::cout << "Your item is : " << perp(length,diameter) << ".\n";
        }
    }
    else if (mode == "reset")
    {
        perp.reset();
    }
    else if (mode == "evolve")
    {
        perp.load();
        double result{network_evolve(perp,generate_data(std::stoi(argv[3])),std::stoi(argv[2]))};
        if (result > 0) perp.save();
    }
}
