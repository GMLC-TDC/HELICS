function v = HELICS_DATA_TYPE_VECTOR()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1593856932);
  end
  v = vInitialized;
end
