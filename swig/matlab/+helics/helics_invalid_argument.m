function v = helics_invalid_argument()
  persistent vInitialized;
  if isempty(vInitialized)
    vInitialized = helicsMEX(0, 1818783839);
  end
  v = vInitialized;
end
