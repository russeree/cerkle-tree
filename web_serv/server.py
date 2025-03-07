from flask import Flask, render_template, request, jsonify
import smt_bindings
import json

app = Flask(__name__)

# Initialize SMT with empty default value
smt = smt_bindings.SMT("0" * 64)  # 32 bytes of zeros in hex

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/api/root', methods=['GET'])
def get_root():
    return jsonify({'root': smt.get_root_hash()})

@app.route('/api/leaf', methods=['GET', 'POST', 'DELETE'])
def handle_leaf():
    if request.method == 'GET':
        key = request.args.get('key')
        if not key:
            return jsonify({'error': 'Key is required'}), 400
        value = smt.get_leaf(key)
        exists = smt.has_leaf(key)
        return jsonify({'value': value, 'exists': exists})
    
    elif request.method == 'POST':
        data = request.get_json()
        key = data.get('key')
        value = data.get('value')
        if not key or not value:
            return jsonify({'error': 'Key and value are required'}), 400
        try:
            smt.set_leaf(key, value)
            return jsonify({'success': True, 'root': smt.get_root_hash()})
        except Exception as e:
            return jsonify({'error': str(e)}), 400
    
    elif request.method == 'DELETE':
        key = request.args.get('key')
        if not key:
            return jsonify({'error': 'Key is required'}), 400
        try:
            smt.remove_leaf(key)
            return jsonify({'success': True, 'root': smt.get_root_hash()})
        except Exception as e:
            return jsonify({'error': str(e)}), 400

@app.route('/api/proof', methods=['GET'])
def get_proof():
    key = request.args.get('key')
    if not key:
        return jsonify({'error': 'Key is required'}), 400
    try:
        proof = smt.generate_proof(key)
        siblings = [proof.get_sibling(i) for i in range(proof.size())]
        return jsonify({
            'siblings': siblings,
            'valid': proof.is_valid()
        })
    except Exception as e:
        return jsonify({'error': str(e)}), 400

@app.route('/api/verify', methods=['POST'])
def verify_proof():
    data = request.get_json()
    key = data.get('key')
    value = data.get('value')
    siblings = data.get('siblings', [])
    
    if not all([key, value, siblings]):
        return jsonify({'error': 'Key, value, and siblings are required'}), 400
    
    try:
        proof = smt_bindings.MerkleProof()
        for sibling in siblings:
            proof.add_sibling(sibling)
        
        is_valid = smt.validate_proof(key, value, proof)
        return jsonify({'valid': is_valid})
    except Exception as e:
        return jsonify({'error': str(e)}), 400

if __name__ == '__main__':
    app.run(debug=True)
