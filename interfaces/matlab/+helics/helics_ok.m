function v = helics_ok()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1398230852);
  end
  v = vInitialized;
end
