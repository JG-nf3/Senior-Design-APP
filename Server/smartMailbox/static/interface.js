class ClickCounter extends React.Component {
    constructor(props) {
        super(props);
        this.state = { clickCount: 0};
    }

    render() {
        const user_id = JSON.parse(document.getElementById('user_id').textContent);
        console.log(user_id)

        return <div className="container">
        <div className="row">
          <div className="col">
            <div className="card">
                <div className="card-body">
                  <h5 className="card-title">Card title</h5>
                  <p className="card-text">Some quick example text to build on the card title and make up the bulk of the card's content.</p>
                </div>
                <ul className="list-group list-group-flush">
                  <li className="list-group-item">An item</li>
                  <li className="list-group-item">A second item</li>
                  <li className="list-group-item">A third item</li>
                </ul>
                <div className="card-body">
                  <a href="#" className="card-link">Card link</a>
                  <a href="#" className="card-link">Another link</a>
                </div>
              </div>
          </div>
          <div className="col">
            Column
          </div>
          <div className="col">
            Column
          </div>
        </div>
      </div>;
    }
}