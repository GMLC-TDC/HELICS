function v = helics_invalid_argument()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 3);
  end
  v = vInitialized;
end
