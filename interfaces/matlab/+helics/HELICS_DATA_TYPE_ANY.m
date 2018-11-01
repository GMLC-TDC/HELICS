function v = HELICS_DATA_TYPE_ANY()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1128095569);
  end
  v = vInitialized;
end
