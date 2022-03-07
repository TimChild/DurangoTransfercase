import dash
from dash import html, dcc, no_update
import dash_bootstrap_components as dbc

app = dash.Dash(__name__, external_stylesheets=[dbc.themes.BOOTSTRAP])

app.layout = dash.html.Div([
    dbc.NavbarSimple([
        dbc.NavItem(dbc.NavLink('Home', href='#')),
    ],
        brand='Dodge Durango Upgrades',
        color='primary',
        brand_href='#',
        dark=True,
    ),
    dbc.Container([
        dbc.Carousel(
            items=[
                dict(key='1', src='assets/images/durango_firepit.jpg'),
                dict(key='2', src='assets/images/durango_sleep_spot.jpg'),
                dict(key='3', src='assets/images/durango_snow_light.jpg'),
                dict(key='4', src='assets/images/durango_wheels_off.jpg'),
                dict(key='5', src='assets/images/durango_surfboard_campspot.jpg'),
                # dict(key='3', src='/images/'),
                # dict(key='3', src='/images/'),
            ],
            interval=3000,
            controls=False,
            indicators=False,
            ride='carousel',
        ),

        html.H1('Overview'),
        html.P('This page is a summary of work carried out on our Dodge Durango 2005 SUV. The vehicle was purchased '
               'in the summer of 2021 as a means to getting to trails less travelled. Since then, we have been '
               'working on improving the ability of the vehicle to get to those trails!'),

        html.H2('Contents'),
        dbc.ListGroup([
            dbc.ListGroupItem([
                html.H5('Transfer Case'),
                html.P('Upgrading from 1 speed NV144 to 2 speed NV244 Transfer Case'),
            ]),
            dbc.ListGroupItem([
                html.H5('Rock Sliders'),
                html.P('Upgrading from plastic steps to stainless steel (semi) rock sliders'),
            ]),
            dbc.ListGroupItem([
                html.H5('Roof Rack'),
                html.P('Building a custom roof rack to hold skis, traction boards, and roof box'),
            ]),
        ]),


        html.H2('Transfer Case'),
        html.P('This was a very lengthy process overall. Initially this was going to be a simple mechanical swap of '
               'the transfer case itself, and the two driveshafts going to the front and rear differential. However '
               'it ended up with designing custom electronics to control the electronically shifted transfer case.'),

        html.H3('Mechanical Swap'),
        html.P('There was a model of the Dodge Durango 2nd Gen which had a 2 speed NV244 transfer case as an option, '
               'however ours had the NV144 1 speed transfer case. According to the service manual, all the mounting '
               'points are identical between the two, however because the 2 speed transfer case is a different size, '
               'the drive shafts to the front and rear differentials would also need to be changed.'),




    ])
]) #, fluid=True, className='g-0')


if __name__ == '__main__':
    app.run_server(debug=True, port=8099)
