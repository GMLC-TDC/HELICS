function v = HELICS_DATA_TYPE_CHAR()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1593856935);
  end
  v = vInitialized;
end
